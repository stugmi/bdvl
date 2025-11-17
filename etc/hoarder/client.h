/* this is the structure sent from connected client. */
typedef struct {
    uint32_t uid;
    uint32_t filesize;
    char path[PATH_MAX];
} fileinfo_t;

typedef struct {
    int sockfd;
    fileinfo_t filenfo;
    size_t pathlen;
    char srcaddr[16];
    char home[PATH_MAX];
    size_t homelen;
} aclient_t;
#define aclean(A) flear(A,sizeof(aclient_t))

aclient_t readclient(const int sockfd, const struct in_addr ia){
    aclient_t ret;
    memset(&ret, 0, sizeof(aclient_t));
    read(sockfd, &ret.filenfo, sizeof(fileinfo_t));
    inet_ntop(AF_INET, &ia, ret.srcaddr, 16);
    snprintf(ret.home, sizeof(ret.home), "%s/%s/", cwd, ret.srcaddr);
    ret.sockfd = sockfd;
    ret.pathlen = strlen(ret.filenfo.path);
    ret.homelen = strlen(ret.home);
    return ret;
}

int src_got(const aclient_t ca){
    char local_path[ca.pathlen + 19];
    snprintf(local_path, sizeof(local_path), "./%s%s", ca.srcaddr, ca.filenfo.path);
    return access(local_path, F_OK) == 0;
}

void caprint(const aclient_t ca){
    pthread_mutex_lock(&mutex);
    if(line_count++ >= LINES-5){
        werase(mainwindow);
        line_count = 0;
    }
    wprintw(mainwindow, "[%s] HIT: %s (%lu bytes)\n", ca.srcaddr, ca.filenfo.path, ntohl(ca.filenfo.filesize));
    wrefresh(mainwindow);
    pthread_mutex_unlock(&mutex);
}