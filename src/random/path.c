int path_is_link(const char *path){
    struct stat sbuf;
    hook(C__LXSTAT);
    memset(&sbuf, 0, sizeof(struct stat));
    if((long)call(C__LXSTAT, _STAT_VER, path, &sbuf) < 0)
        return -1;
    return S_ISLNK(sbuf.st_mode);
}

char *get_valid_rootdir(const char *rdir, DIR **rdp){
    DIR *dp = NULL;
    char *rootdir = NULL;
    u_short errcount = 0;

    hook(COPENDIR);

    while(dp == NULL){
        while(rootdir == NULL){
            if(errcount > 25)
                return NULL;

            if(rdir == NULL)
                rootdir = xordup(rootdirs[rand() % ROOTDIRS_SIZE]);
            else rootdir = strdup(rdir);
            if(!rootdir){
                ++errcount;
                continue;
            }

            /* if a rootdir has been randomly selected, don't try to use it if its a symlink. */
            int islink = path_is_link(rootdir);
            if((rdir == NULL && islink) || islink < 0){
                /* this isn't an error. just go again. */
                clean(rootdir);
                rootdir = NULL;
                continue;
            }
        }

        if((dp = call(COPENDIR, rootdir)) == NULL){
            clean(rootdir);
            rootdir = NULL; // get new rootdir. this one don't work.
        }
    }

    *rdp = dp != NULL ? dp : NULL;
    return rootdir;
}

char *select_random_dir(DIR *dp){
    struct dirent *dir;
    char dirslist[MAXDIRS][128];
    size_t count = 0;
    hook(CREADDIR);
    while((dir = call(CREADDIR, dp)) != NULL && count < MAXDIRS){
        if(*dir->d_name != '.'){
            strncpy(dirslist[count], dir->d_name, sizeof(dirslist[count])-1);
            ++count;
        }
    }
    return strdup(dirslist[rand() % count]);
}

/* returns a random, new, path. */
char *randpath(const size_t len, const char *rdir){
    char *rootdir, *randdir;
    DIR *dp;

    rootdir = get_valid_rootdir(rdir, &dp);
    if(!rootdir){
        if(dp) closedir(dp);
        return NULL;
    }else if(!dp){
        if(rootdir) clean(rootdir);
        return NULL;
    }else{
        /* get_valid_rootdir was successful. (try to) fetch a random directory name from dp. */
        randdir = select_random_dir(dp);
        closedir(dp);

        if(!randdir){
            clean(rootdir);
            return NULL;
        }
    }

    char buf[strlen(rootdir) + strlen(randdir) + 64];
    int s = snprintf(buf, sizeof(buf)-1, "%s/%s", rootdir, randdir);
    clean(rootdir);
    clean(randdir);
    if(s <= 0)
        return NULL;

    /* path must adhere to specified len. */
    get_lower_charset();
    size_t count = 0;
    while(s + count < len)
        buf[s + count++] = randchar(buf_lowercase);
    for(u_int i = 3; i > 0; i--) // change the end up a little
        buf[len - i] = randchar(buf_lowercase);
    buf[len] = '\0';

    if(pathexists(buf))
        return randpath(len, rdir); // go again

    return strdup(buf);
}