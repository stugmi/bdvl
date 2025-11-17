void get_symbol_pointer(int index, void *handle){
    if(symbols[index] != NULL || all_calls[index] == NULL)
        return;

    char *symbol_name;
    void *fptr;

    locate_dlsym();
    
    symbol_name = xordup(all_calls[index]);
    fptr = o_dlsym(handle, symbol_name);
    clean(symbol_name);
    
    if(!fptr) return;
    symbols[index] = fptr;
}

/* this function has a wrapper macro called hook in libdl.h.
 * naming this function & its subsequent macro 'hook' is a
 * little inaccurate. but that matters not. */
void _hook(void *handle, ...){
    int index;
    va_list va;

    va_start(va, handle);
    while((index = va_arg(va, int)) > -1 && index < ALL_CALLS_SIZE)
        get_symbol_pointer(index, handle);
    va_end(va);
}
