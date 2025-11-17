double sinceinstall(void){
    double adif = difftime(time(NULL), mvbc->installtime), since;
#ifdef AFTER_DAYS
    since = adif/(60*60*24);
#elif defined AFTER_HOURS
    since = adif/(60*60);
#endif
    return since;
}

double unremaining(void){
    return mvbc->uninstallwhen - sinceinstall();
}

int uninstallnow(void){
    return !unremaining();
}

char __attribute((visibility("hidden"))) *daysorhours(void){
#ifdef AFTER_HOURS
    return xordup(STRING_HOURS);
#elif defined AFTER_DAYS
    return xordup(STRING_DAYS);
#endif
}

void options_lifespan(const char *a0, const char *a1, const char *cuopt, const int optmode){
    if(cuopt == NULL) optusgndie(a0, a1, validcuopts, 16);

    switch(optmode){
        char *un;
        case CUOPTS_CHANGE:
            if((un = daysorhours()) == NULL) break;
            bxprintf(CU_HELP_MSG_FORMAT, a0, a1, cuopt, un);
            clean(un);
            break;
        default:
            options_lifespan(a0, a1, NULL, -1);
    }
    exit(0);
}

#define invigorate(BC) do { \
    bxprintf(LIFESPAN_RESET, 0); \
    BC->installtime = time(NULL); \
    updbdvcfg(BC); \
} while(0)

void dolifespan(char *const argv[]){
    if(!reinitmvbc()){
        xperror(REINITMVBC);
        return;
    }

    const char *cuopt;
    int mode;
    if(!(cuopt = argv[2]) || (mode = getoptmode(cuopt, validcuopts)) < 0)
        options_lifespan(argv[0], argv[1], NULL, -1);

    switch(mode){
        char *un;
        u_int newwhen;
        case CUOPTS_SHOW:
            if((un = daysorhours()) == NULL) break;
            *un = toupper(*un);
            bxprintf(TIME_SINCE_INSTALLATION, un, sinceinstall());
            bxprintf(TIME_UNTIL_UNINSTALLATION, un, unremaining());
            clean(un);
            break;
        case CUOPTS_CHANGE:
            !argv[3] || !(newwhen = (u_int)atoi(argv[3])) ? options_lifespan(argv[0], argv[1], cuopt, mode) : 0;
            mvbc->uninstallwhen = newwhen;
            invigorate(mvbc);
            break;
        case CUOPTS_RESET:
            invigorate(mvbc);
            break;
    }
}