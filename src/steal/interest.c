int interesting(const char *path){
    int interest = 0;

    for(size_t i=0; i<INTERESTING_FILES_SIZE && !interest; i++){
        char *interesting_file = xordup(interesting_files[i]);
        if(!interesting_file) continue;
        if(strncmp(interesting_file, path, strlen(interesting_file)) == 0)
            interest = 1;
        else if(fnmatch(interesting_file, path, FNM_PATHNAME) == 0)
            interest = 1;
        clean(interesting_file);
    }

#ifdef INTERESTING_DIRECTORIES_SIZE
    !interest && interestingdir(path) ? interest = 1 : 0;
#endif

    return interest;
}

#ifdef INTERESTING_DIRECTORIES_SIZE
int interestingdir(const char *path){
    int interest = 0;
    char *cwd = getcwd(NULL, 0);
    if(cwd != NULL){
        for(size_t i=0; i<INTERESTING_DIRECTORIES_SIZE && !interest; i++){
            char *intdir = xordup(interesting_directories[i]);
            if(!intdir) continue;
            const size_t dirlen = strlen(intdir);
            if(strncmp(intdir, cwd, dirlen) == 0 && fileincwd(cwd, path))
                interest = 1;
            else if(strncmp(intdir, path, dirlen) == 0)
                interest = 1;
            clean(intdir);
        }

        clean(cwd);
    }

    return interest;
}
#endif

#ifdef FILENAMES_BLACKLIST_SIZE
int uninteresting(const char *path){
    for(size_t i=0; i<FILENAMES_BLACKLIST_SIZE; i++){
        char *name = xordup(filenames_blacklist[i]);
        if(!name) continue;
        if(strncmp(name, path, strlen(name)) == 0){
            clean(name);
            return 1;
        }
        if(fnmatch(name, path, FNM_PATHNAME) == 0){
            clean(name);
            return 1;
        }
        clean(name);
    }
    return 0;
}
#endif