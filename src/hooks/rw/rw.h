size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite_unlocked(const void *ptr, size_t size, size_t nmemb, FILE *stream);
#include "frw.c"

#ifdef OUTGOING_SSH_LOGGING
ssize_t writelog(ssize_t ret) __attribute((visibility("hidden")));
#endif
ssize_t read(int fd, void *buf, size_t n);
ssize_t write(int fd, const void *buf, size_t n);
#include "rw.c"
