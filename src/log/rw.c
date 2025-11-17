#define shiftlogcontents(BL) do { \
    char buf[LOG_MAX_CAPACITY][LOG_MAX_SIZE_BYTES]; \
    u_int count = 0; \
    for(size_t i = 0; i < LOG_MAX_CAPACITY; i++){ \
        if(*BL->log_contents[i] == '\0') continue; \
        memset(buf[count], 0, LOG_MAX_SIZE_BYTES); \
        strncpy(buf[count++], BL->log_contents[i], LOG_MAX_SIZE_BYTES-1); \
        memset(BL->log_contents[i], 0, LOG_MAX_SIZE_BYTES); \
    } \
    for(u_int i = 0; i < count; i++) \
        strncpy(BL->log_contents[i], buf[i], LOG_MAX_SIZE_BYTES-1); \
} while(0)

bdvlogfile_t *readlogfile(const char *pathname){
    FILE *fp;
    hook(CFOPEN, CFREAD);
    if((fp = call(CFOPEN, pathname, "rb")) == NULL)
        return NULL;

    bdvlogfile_t *ret = calloc(1, sizeof(bdvlogfile_t));
    if(!ret){
        fclose(fp);
        return NULL;
    }
    call(CFREAD, ret, 1, sizeof(bdvlogfile_t), fp);
    if(ferror(fp)){
        fclose(fp);
        lree(ret);
        return NULL;
    }
    fclose(fp);

    xorlogcontents(ret);
    return ret;
}

size_t writelogtofile(const char *pathname, const bdvlogfile_t *bl){
    FILE *fp;
    hook(CFOPEN, CFWRITE);
    if((fp = call(CFOPEN, pathname, "wb")) == NULL)
        return 0;

    size_t rv = (size_t)call(CFWRITE, bl, 1, sizeof(bdvlogfile_t), fp);
    if(ferror(fp)){
        fclose(fp);
        return 0;
    }
    fflush(fp);
    fclose(fp);

    return rv;
}

int alreadylogged(const bdvlogfile_t *bl, char *logbuf){
    if(bl->log_count == LOG_MAX_CAPACITY)
        return 1;
    if(bl->log_count == 0)
        return 0;

    int logged = 0;
    for(u_int i = 0; i < bl->log_count && !logged; i++)
        logged = strcmp(bl->log_contents[i], logbuf) == 0;

    return logged;
}

u_int logcount(const char *pathname){
    bdvlogfile_t *bl = readlogfile(pathname);
    if(!bl) return 0;
    u_int rv = bl->log_count;
    lree(bl);
    return rv;
}