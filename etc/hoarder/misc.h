void usgndie(const char *a0, const char *cwd){
    printf("\n \e[1m~\e[0m \e[1;31mHOARDER\e[0m \e[1m~\e[0m \n\n");
    printf(" Usage: %s <port>\n", a0);
    printf("  Specified port \e[1mmust be a hidden bdvl port\e[0m,\n");
    printf("  otherwise no files will be received.\n");
    printf(" Default output directory: . (%s)\n\n", cwd);
    exit(-1);
}

#define sizeofarr(A) sizeof(A)/sizeof(A[0])

#define flear(PTR,SIZE) do { \
    memset(PTR,0,SIZE); \
    free(PTR); \
} while(0)
#define clean(S) flear(S,strlen(S))

int direxists(const char *pathname){
    const size_t pathsize = strlen(pathname),
                 tmpsize = pathsize+2;
    char tmp[tmpsize];
    memset(tmp, 0, tmpsize);
    strcpy(tmp, pathname);
    if(tmp[pathsize - 1] != '/')
        strcat(tmp, "/");
    return access(tmp, F_OK) == 0;
}

u_long getfilesize(const char *path){
    struct stat sbuf;
    memset(&sbuf, 0, sizeof(struct stat));
    if(stat(path, &sbuf) < 0 || sbuf.st_size == 0)
        return 0;
    return (u_long)sbuf.st_size;
}

char *pathtmp(char *newpath){
    const size_t pathsize = strlen(newpath) + 5;
    char *path = malloc(pathsize);
    if(!path) return NULL;
    snprintf(path, pathsize, "%s.tmp", newpath);
    return path;
}

int tmpup(char *newpath){
    if(access(newpath, F_OK) != 0)
        return 0;

    char *path = pathtmp(newpath);
    if(path == NULL) return -1;

    int status = 0;
    if(access(path, F_OK) != 0){
        clean(path);
        return 0;
    }

    const off_t o = getfilesize(newpath);
    usleep(75000);
    const off_t n = getfilesize(newpath);

    if(o == n) unlink(path);
    else if(n > o) status = 1;

    clean(path);
    return status;
}