
u_long get_blocksize(u_long filesize, u_short *oneshort){
    u_long c = filesize - 1;
    while(filesize % c != 0) --c;
    *oneshort = c == 1 ? 1 : 0;
    return c;
}

/* this is called by each thread per client. */
void proc_aclient(aclient_t ca){
    size_t ndirs;
    char **newdirs = getdirstructure(ca.filenfo.path, &ndirs);
    if(!newdirs) return;

    char *newpath = createdirstructure(ca, newdirs, ndirs), *newpathdup;
    if(!newpath){
        freedirs(newdirs, ndirs);
        return;
    }

dodo: /* if the directory in which newpath resides does not exist, it needs to be created now. */
    newpathdup = strdup(newpath);
    if(!newpathdup){
        clean(newpath);
        freedirs(newdirs, ndirs);
        return;
    }
    char *pathd = dirname(newpathdup);
    int does_exist = direxists(pathd);
    clean(newpathdup);
    if(!does_exist && errno == ENOENT){
        if(mkdirstructure(ca, newdirs, ndirs) < 0){
            clean(newpath);
            freedirs(newdirs, ndirs);
            return;
        }
        goto dodo;
    }else{
        freedirs(newdirs, ndirs);
        if(!does_exist){
            clean(newpath);
            return;
        }
    }

    int t = tmpup(newpath);
    if(t != 0){
        clean(newpath);
        return;
    }

    struct stat sbuf;
    memset(&sbuf, 0, sizeof(struct stat));
    int statr = stat(newpath, &sbuf);
    if(statr < 0 && errno != ENOENT){
        clean(newpath);
        return;
    }else if(statr == 0 && sbuf.st_size == (off_t)ntohl(ca.filenfo.filesize)){
        clean(newpath);
        return;
    }

    unlink(newpath);
    FILE *out_fp = fopen(newpath, "wb");
    if(out_fp == NULL){
        clean(newpath);
        return;
    }

    char *ptmp = pathtmp(newpath);
    clean(newpath);
    if(!ptmp){
        fclose(out_fp);
        return;
    }

    int cr = creat(ptmp, 0600);
    if(cr < 0){
        fclose(out_fp);
        unlink(ptmp);
        clean(ptmp);
        return;
    }else close(cr);

    u_short oneshort;
    u_long file_blocksize = get_blocksize(ntohl(ca.filenfo.filesize), &oneshort);
    if(oneshort){
        u_char tmp[1];
        read(ca.sockfd, tmp, 1);
        fputc(*tmp, out_fp);
    }

    u_char *inbuffer = malloc(file_blocksize);
    if(inbuffer != NULL){
        ssize_t bytes_in;
        while((bytes_in = read(ca.sockfd, inbuffer, file_blocksize)) > 0)
            fwrite(inbuffer, 1, bytes_in, out_fp);
        free(inbuffer);
    }

    fclose(out_fp);
    unlink(ptmp);
    clean(ptmp);
}

static void *client_handler(void *arg){
    (void)arg;
    while(1){
        aclient_t ca = queue_get();
        caprint(ca);
        ++active_threads;
        proc_aclient(ca);
        shutdown(ca.sockfd, SHUT_RDWR);
        close(ca.sockfd);
        --active_threads;
    }
    return NULL;
}

static void *titlebar_handler(void *arg){
    hoarder_t *hmp = (hoarder_t*)arg;

    WINDOW *titlebar = newwin(2, COLS, 0, 0);
    scrollok(titlebar, TRUE);
    updtitlebar(titlebar, hmp->port, q, active_threads);

    while(1){
        usleep(75000);
        updtitlebar(titlebar, hmp->port, q, active_threads);
    }
    delwin(titlebar);

    return NULL;
}