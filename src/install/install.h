#include "cfg/cfg.h"
#include "inject/inject.h"
#include "ldpatch/ldpatch.h"
#include "install.c"
#include "prep.c"
#include "reinstall.c"
#ifdef SEL_CONFIG
#include "selinux.c"
#endif
#include "so.c"
#include "uninstall.c"
#include "update.c"