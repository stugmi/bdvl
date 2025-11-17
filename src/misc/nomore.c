int _rknomore(const char *installdir, const char *bdvlso){
    hook(COPENDIR, CREADDIR);

    DIR *dp = call(COPENDIR, installdir);
    if(dp == NULL) return 1;

    const size_t solen = strlen(bdvlso);
    int status = 0;
    struct dirent *dir;

    while((dir = call(CREADDIR, dp)) != NULL && !status)
        if(*dir->d_name != '.')
            status = strncmp(bdvlso, dir->d_name, solen) == 0;

    closedir(dp);
    return !status;
}