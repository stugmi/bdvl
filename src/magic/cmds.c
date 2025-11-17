/* there are macros for these functions in bdv.c to simplify usage, under the respective function prototype. */

char *_buildbdvoptargs(char buf[], const size_t bufsize, char *const *opts, const size_t arrsize, const char sep){
    size_t cursize = 0;
    memset(buf, 0, bufsize);
    for(size_t i = 0; i < arrsize; i++){
        if(!opts[i]) continue;
        char *opt = xordup(opts[i]);
        if(!opt) return 0;
        if(*opt == '\0'){
            clean(opt);
            continue;
        }
        const size_t tmpsize = strlen(opt)+1;
        if(cursize+tmpsize >= bufsize){
            clean(opt);
            break;
        }
        char tmp[tmpsize+1];
        snprintf(tmp, sizeof(tmp), "%s%c", opt, sep);
        clean(opt);
        strncat(buf, tmp, bufsize);
        cursize += tmpsize;
    }
    buf[cursize-1] = '\0';
    return buf;
}

int _getoptmode(const char *gopt, char *const *opts, const size_t arrsize){
    int mode = -1;
    for(size_t i = 0; i < arrsize && mode < 0; i++){
        char *opt = xordup(opts[i]);
        if(!opt) return -1;
        if(strchr(opt, ' ') != NULL){
            char *d = strtok(opt, " ");
            if(d && strcmp(d, gopt) == 0)
                mode = i;
        }else if(strcmp(opt, gopt) == 0) mode = i;
        clean(opt);
    }
    return mode;
}

void _optusgndie(const char *a0, const char *a1, char *const *opts, const size_t arrsize, const size_t maxelemsize){
    char b[arrsize*maxelemsize];
    printf("  %s %s %s\n", a0, a1, _buildbdvoptargs(b, sizeof(b), opts, arrsize, '/'));
    exit(0);
}