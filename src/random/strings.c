/* intended for returning a random environment variable, all uppercase, of length len. */
char *randvar(const size_t len){
    get_upper_charset();

    size_t i;
    char buf[len + 1];
    memset(buf, 0, sizeof(buf));
    for(i=0; i < len; i++)
        buf[i] = randchar(buf_uppercase);
    buf[i] = '\0';
    return strdup(buf);
}

/* returns a random garbage string of length len consisting of upper & lowercase characters. */
char *randgarb(const size_t len){
    get_upper_charset();
    get_lower_charset();

    const char *charsets[] = {buf_uppercase, buf_lowercase};
    const size_t sc = sizeofarr(charsets);
    size_t i = 0;
    char buf[len + 1];

    srand(time(NULL));
    memset(buf, 0, len + 1);
    for(i=0; i < len; i++)
        buf[i] = randchar(charsets[rand() % sc]);
    buf[i] = '\0';

    return strdup(buf);
}

/* returns a new soname based on the name of the installdir.
 * if the installdir's name does not have 'lib' at the beginning of it, the resulting soname gets 'lib' prepended to it. */
char *randso(const char *installdir){
    char *dupdup, *bname;

    dupdup = xordup(installdir);
    if(!dupdup) return NULL;
    bname = basename(dupdup);
    if(!bname){
        clean(dupdup);
        return NULL;
    }
    const size_t rs = strlen(bname)+1;

    char *ret = malloc(rs+6);
    if(!ret){
        clean(dupdup);
        return NULL;
    }else memset(ret, 0, rs+6);

    if(strncmp("lib", bname, 3) != 0){
        ret[0] = 'l';
        ret[1] = 'i';
        ret[2] = 'b';
        strncpy(ret+3, bname, rs);
    }else strncpy(ret, bname, rs);
    clean(dupdup);
    strcat(ret, ".so");
    return ret;
}