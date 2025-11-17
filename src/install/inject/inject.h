#ifndef INJECT_H
#define INJECT_H

#define PTRACE_PEEKTEXT 1
#define PTRACE_POKETEXT 4
#define PTRACE_CONT 7
  /* Get all general purpose registers used by a processes.
     This is not supported on all machines.  */
#define PTRACE_GETREGS 12
  /* Set all general purpose registers used by a processes.
     This is not supported on all machines.  */
#define PTRACE_SETREGS 13
  /* Attach to a process that is already running. */
#define PTRACE_ATTACH 16
  /* Detach from a process attached to with PTRACE_ATTACH.  */
#define PTRACE_DETACH 17

#define INJERR_SUCCESS 0
#define INJERR_OTHER -1
#define INJERR_NO_MEMORY -2
#define INJERR_NO_PROCESS -3
#define INJERR_NO_LIBRARY -4
#define INJERR_NO_FUNCTION -4
#define INJERR_ERROR_IN_TARGET -5
#define INJERR_FILE_NOT_FOUND -6
#define INJERR_INVALID_MEMORY_AREA -7
#define INJERR_PERMISSION -8
#define INJERR_UNSUPPORTED_TARGET -9
#define INJERR_INVALID_ELF_FORMAT -10
#define INJERR_WAIT_TRACEE -11

#ifdef __LP64__
#define SIZE_T_FMT "l"
#else
#define SIZE_T_FMT ""
#endif

#define PTRACE_OR_RETURN(request, injector, addr, data) do{ \
    int rv = injector__ptrace(request, injector->pid, addr, data, #request); \
    if(rv != 0) return rv; \
}while(0)

#include "elf.c"
#include "ptrace.c"
#include "remote_call.c"
#include "inject.c"
#include "injector.c"



#endif