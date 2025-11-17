int open(const char *pathname, int flags, mode_t mode);
int open64(const char *pathname, int flags, mode_t mode);
#include "open.c"

FILE *fopen(const char *pathname, const char *mode);
FILE *fopen64(const char *pathname, const char *mode);
#include "fopen.c"

int access(const char *pathname, int amode);
int creat(const char *pathname, mode_t mode);
#include "access.c"