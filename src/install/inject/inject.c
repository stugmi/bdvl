int injector_attach(injector_t **injector_out, pid_t pid){
    injector_t *injector;
    int status, rv = 0;
    long retval;

    injector = calloc(1, sizeof(injector_t));
    if(injector == NULL) return INJERR_NO_MEMORY;
    
    injector->pid = pid;
    rv = injector__attach_process(injector);
    if(rv != 0) goto error_exit;
    injector->attached = 1;

    while((rv = waitpid(pid, &status, 0)) == -1 && errno == EINTR);
    if(rv < 0){
        rv = INJERR_WAIT_TRACEE;
        goto error_exit;
    }

    rv = injector__collect_libc_information(injector);
    if(rv != 0) goto error_exit;

    rv = injector__get_regs(injector, &injector->regs);
    if(rv != 0) goto error_exit;

    rv = injector__read(injector, injector->code_addr, &injector->backup_code, sizeof(injector->backup_code));
    if(rv != 0) goto error_exit;

    injector->text_size = sysconf(_SC_PAGESIZE);
    injector->stack_size = 2 * 1024 * 1024;

    rv = injector__call_syscall(injector, &retval, injector->sys_mmap, 0,
                                injector->text_size + injector->stack_size, PROT_READ,
                                MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
    if(rv != 0) goto error_exit;
    if(retval == -1){
        rv = INJERR_ERROR_IN_TARGET;
        goto error_exit;
    }

    injector->mmapped = 1;
    injector->text = (size_t)retval;
    injector->stack = injector->text + injector->text_size;
    rv = injector__call_syscall(injector, &retval, injector->sys_mprotect,
                                injector->stack, injector->stack_size,
                                PROT_READ | PROT_WRITE);
    if(rv != 0) goto error_exit;
    if(retval != 0){
        rv = INJERR_ERROR_IN_TARGET;
        goto error_exit;
    }

    *injector_out = injector;
    return 0;
error_exit:
    injector_detach(injector);
    return rv;
}

int injector_inject(injector_t *injector, const char *path, void **handle){
    char abspath[PATH_MAX];
    size_t len;
    int rv;
    long retval;

    if(realpath(path, abspath) == NULL)
        return INJERR_FILE_NOT_FOUND;

    len = strlen(abspath)+1;
    if(len > injector->text_size)
        return INJERR_FILE_NOT_FOUND;

    rv = injector__write(injector, injector->text, abspath, len);
    if(rv != 0) return rv;

    rv = injector__call_function(injector, &retval, injector->dlopen_addr, injector->text, RTLD_LAZY);
    if(rv != 0) return rv;
    if(retval == 0) return INJERR_ERROR_IN_TARGET;
    if(handle != NULL) *handle = (void*)retval;
    return 0;
}

int injector_uninject(injector_t *injector, void *handle){
    int rv;
    long retval;

    rv = injector__call_function(injector, &retval, injector->dlclose_addr, handle);
    if(rv != 0) return rv;
    if(retval != 0) return INJERR_ERROR_IN_TARGET;
    return 0;
}

int injector_detach(injector_t *injector){
    if(injector->mmapped)
        injector__call_syscall(injector, NULL, injector->sys_munmap, injector->text, injector->text_size + injector->stack_size);
    if(injector->attached)
        injector__detach_process(injector);
    bfree(injector, sizeof(injector_t));
    return 0;
}