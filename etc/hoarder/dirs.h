#define MAXDIRS 32 // maximum directory names which can be stored from a given path.

void freedirs(char **dirs, const size_t ndirs){
    for(size_t i = 0; i < ndirs; i++)
        clean(dirs[i]);
    free(dirs);
}

/* iterates through each directory component in path & stores each individually in an array.
 * resulting array is returned at the end. ndirs is updated with the dir count before returning. */
char **getdirstructure(const char *path, size_t *ndirs){
    char **dirs = calloc(MAXDIRS, sizeof(char*));
    if(!dirs) return NULL;

    char *targetpath;
    if((targetpath = strdup(path)) == NULL){
        free(dirs);
        return NULL;
    }

    /* copy & store each directory component in targetpath. */
    char *p = strtok(targetpath, "/");
    *ndirs = 0;
    while(p != NULL && *ndirs < MAXDIRS){
        dirs[*ndirs] = strdup(p);
        if(!dirs[*ndirs]){
            clean(targetpath);
            *ndirs != 0 ? freedirs(dirs, *ndirs) : 0;
            return NULL;
        }else ++*ndirs;
        p = strtok(NULL, "/");
    }
    clean(targetpath);

    return dirs;
}

char *createdirstructure(const aclient_t ca, char **dirs, const size_t ndirs){
    char fullpath[ca.homelen + ca.pathlen + 128];
    size_t buflen = 0;

    memset(fullpath, 0, sizeof(fullpath));
    strcat(fullpath, ca.home);
    buflen = ca.homelen + 1;

    for(size_t i = 0; i < ndirs-1; i++){
        const size_t tmpsize = strlen(dirs[i]) + 2;
        if(tmpsize+buflen >= sizeof(fullpath)-1)
            break;

        char tmp[tmpsize];
        snprintf(tmp, tmpsize, "%s/", dirs[i]);
        strcat(fullpath, tmp); 
        buflen += tmpsize-1;
    }
    fullpath[buflen - 1] = '\0';

    char *pathdup = strdup(ca.filenfo.path),
         *filename = basename(pathdup);
    strcat(fullpath, filename);
    fullpath[buflen + strlen(filename)] = '\0';
    clean(pathdup);

    return strdup(fullpath);
}

/* within home (INTEREST_DIR) make all directories for the directory structure which is based on the contents of the dirs array.
 * if the calling process is root & has created a new directory within the specified structure, it will chown 0:0 $path. otherwise, the magicid would be left behind. */
int mkdirstructure(const aclient_t ca, char **dirs, const size_t ndirs){
    char current[PATH_MAX];
    size_t cursize;

    if(!direxists(ca.home))
    	mkdir(ca.home, 0777);

    memset(current, 0, sizeof(current));
    strcat(current, ca.home);
    current[ca.homelen] = '\0';
    cursize = ca.homelen + 1;

    for(size_t i = 0; i < ndirs-1; i++){
        const size_t tmpsize = strlen(dirs[i]) + 2;
        if(tmpsize+cursize >= sizeof(current)-1)
            break;

        char tmp[tmpsize];
        snprintf(tmp, tmpsize, "%s/", dirs[i]);
        strcat(current, tmp);
        cursize += tmpsize-1;

        int mr = mkdir(current, 0777);
        if(mr < 0 && errno != EEXIST){
        	perror("mkdir");
        	return -1;
        }
    }

    return 1;
}