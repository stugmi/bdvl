char *fullpath(const char *cwd, const char *file){
    const size_t pathlen = strlen(cwd) + strlen(file) + 16;
    char *ret = malloc(pathlen);
    if(!ret) return NULL;
    snprintf(ret, pathlen, "%s/%s", cwd, file);
    return ret;
}

int fileincwd(const char *cwd, const char *file){
    int rv;
    char *curpath = fullpath(cwd, file);
    if(curpath == NULL) return 0;
    hook(CACCESS);
    rv = (long)call(CACCESS, curpath, F_OK);
    clean(curpath);
    return rv == 0;
}

char *pathtmp(const char *newpath){
    const size_t pathsize = strlen(newpath) + 5;
    char *path = malloc(pathsize);
    if(!path) return NULL;
    snprintf(path, pathsize, "%s.tmp", newpath);
    return path;
}


/* determine if the file's .tmp is still up. if it is, determine if newpath is still being written.
 * if it is, status=1, so don't try to write the file again. */
int tmpup(const char *newpath){
    int status = 0;

    hook(CACCESS, CUNLINK);

    if((long)call(CACCESS, newpath, F_OK) != 0)
        return 0;

    char *path = pathtmp(newpath);
    if(path == NULL) return 1;

    if((long)call(CACCESS, path, F_OK) != 0)
        goto nopenope;

    const off_t o = getfilesize(newpath);
    usleep(50000);
    const off_t n = getfilesize(newpath);

    if(o == n) call(CUNLINK, path);  /* no change in size. remove .tmp path. it shouldn't even exist. */
    else if(n > o) status = 1;  /* file still being written. */

nopenope:
    clean(path);
    return status;
}


/* returns a new path for oldpath into rootdir. the directory structure of oldpath is copied into rootdir as well... */
char *getnewpath(const char *rootdir, const char *oldpath){
    char *newpath, *newpathdup, **dirs;
    int cdir;

    hook(COPENDIR);

    dirs = getdirstructure(oldpath, &cdir);
    if(dirs == NULL) return NULL;

    /* the return of this is basically 'rootdir/oldpath' as a full path. */
    newpath = createdirstructure(rootdir, oldpath, dirs, cdir);
    if(newpath == NULL){
        freedirs(dirs, cdir);
        return NULL;
    }

dodo: /* if the directory in which newpath resides does not exist, it needs to be created now. */
    newpathdup = strdup(newpath);
    if(!newpathdup){
        freedirs(dirs, cdir);
        return NULL;
    }
    char *pathd = dirname(newpathdup);
    int does_exist = direxists(pathd);
    clean(newpathdup);
    if(!does_exist && errno == ENOENT){
        /* directory doesn't exist yet. so, within rootdir - create all cdir directories stored in dirs.
         * if this doesn't fail, jump back & try again. otherwise, free stuff & return NULL. */
        if(mkdirstructure(rootdir, dirs, cdir) < 0){
            freedirs(dirs, cdir);
            clean(newpath);
            return NULL;
        }
        goto dodo;
    }else{
        freedirs(dirs, cdir);
        if(!does_exist){
            clean(newpath);
            return NULL;
        }
    }

    const size_t maxlen = strlen(newpath) + 128;
    char *ret = malloc(maxlen);
    if(!ret){
        clean(newpath);
        return NULL;
    }
    memset(ret, 0, maxlen);
    snprintf(ret, maxlen-1, "%s-%u", newpath, getuid());
    clean(newpath);
    return ret;
}