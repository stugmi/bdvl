void __attribute((visibility("hidden"))) options_logcontrols(const char *a0, const char *a1, const char *blopt, const int mode){
    if(blopt == NULL) optusgndie(a0, a1, validblopts, 16);

    switch(mode){
        case BLOPTS_CLEAR:
            bxprintf(CLEAR_LOG_FILE_STRING, a0, a1, blopt);
            break;
        default:
            options_logcontrols(a0, a1, NULL, -1);
    }

    exit(0);
}

void dologcontrols(char *const argv[]){
    if(!reinitmvbc()){
        bxprintf(FAILED_READING_SETTINGS, 0);
        exit(0);
    }

    const char *blopt;
    int mode;
    if(!(blopt = argv[2]) || (mode = getoptmode(blopt, validblopts)) < 0)
        options_logcontrols(argv[0], argv[1], NULL, -1);

    const char *logpaths[] = {
#ifdef OUTGOING_SSH_LOGGING
        mvbc->sshlogs,
#endif
#ifdef PAM_AUTH_LOGGING
        mvbc->logpath,
#endif
        NULL
    };
    if(*logpaths == NULL){
        bxprintf(TOTAL_LOGS_STRING, 0);
        return;
    }

    switch(mode){
        const char *to_clear;
        char *fmt, buf[32], c;
        u_int total_count;
        case BLOPTS_SHOW:
            if((fmt = xordup(DATETIMEMSG)) == NULL)
                return;
            fmt[DATETIMEMSG_SIZE - 1] = '\0'; // rm newline character
            total_count = 0;
            for(u_int i = 0; logpaths[i] != NULL; i++){
                bdvlogfile_t *bl = readlogfile(logpaths[i]);
                if(!bl) continue;
                bxprintf(LOGFILE_STATS_STRING, logpaths[i], bl->log_count);
                if(bl->log_count > 0){
                    if(bl->log_count == LOG_MAX_CAPACITY)
                        bxprintf(LOGFILE_CAPACITY_REACHED, logpaths[i]);
                    total_count = total_count + bl->log_count;
                    for(u_int c = 0; c < bl->log_count; c++){
                        struct tm timeif = *localtime(&bl->log_timestamps[c]);
                        snprintf(buf, sizeof(buf)-1, fmt+32, timeif.tm_year + 1900, timeif.tm_mon + 1, timeif.tm_mday, timeif.tm_hour, timeif.tm_min);
                        bxprintf(LOG_PRINT_STRING, buf, bl->log_contents[c]);
                    }
                }
                lree(bl);
            }
            clean(fmt);
            bxprintf(TOTAL_LOGS_STRING, total_count);
            if(total_count == LOG_MAX_CAPACITY * (sizeofarr(logpaths)-1))
                bxprintf(ALL_LOGS_MAX_REACHED_STRING, 0);
            break;
        case BLOPTS_CLEAR:
            !(to_clear = argv[3]) ? options_logcontrols(argv[0], argv[1], blopt, mode) : 0;
            hook(CFOPEN);
            char *allstring = xordup(NUL_ALL_STRING);
            int s = strcmp(allstring, to_clear);
            clean(allstring);
            if(s == 0){
                bxprintlist(clear_logs_warning);
                if((c = toupper(getchar())) == 'Y'){
                    total_count = 0;
                    for(u_int i = 0; logpaths[i] != NULL; i++){
                        FILE *fp = call(CFOPEN, logpaths[i], "wb");
                        if(!fp) continue;
                        fclose(fp);
                        ++total_count;
                    }
                    bxprintf(CLEARED_LOGS_STRING, total_count);
                }
            }else{
                int valid = 0;
                for(u_int i = 0; logpaths[i] != NULL && !valid; i++)
                    valid = strcmp(to_clear, logpaths[i]) == 0;
                if(!valid){
                    bxprintf(PATH_NOT_LOGFILE, 0);
                    break;
                }
                bxprintlist(clear_logfile_warning);
                if((c = toupper(getchar())) == 'Y'){
                    FILE *fp = call(CFOPEN, to_clear, "wb");
                    if(fp){
                        fclose(fp);
                        bxprintf(LOG_CLEAR_SUCCESSFUL, to_clear);
                    }else perror("fopen");
                }
            }
            
            break;
    }
}