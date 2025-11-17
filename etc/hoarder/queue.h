typedef struct {
    aclient_t *clients;
    off_t capacity;
    off_t size;
    off_t front;
    off_t rear;
} queue_t;
queue_t *q;
#define qlean(Q) flear(Q,sizeof(queue_t))

queue_t *initqueue(size_t maxsize){
    queue_t *nq = calloc(1, sizeof(queue_t));
    if(!nq) return NULL;

    nq->clients = calloc(maxsize, sizeof(aclient_t));
    if(!nq->clients){
        free(nq);
        return NULL;
    }

    nq->size = 0,  nq->capacity = maxsize;
    nq->front = 0, nq->rear = -1;
    
    return nq;
}

#define qfull(Q) (Q->size == Q->capacity)
#define qempty(Q) (Q->size == 0)
#define qpush(Q,C) do { \
    if(!qfull(Q)){ \
        ++Q->size; \
        ++Q->rear; \
        if(Q->rear == Q->capacity) \
            Q->rear = 0; \
        Q->clients[Q->rear] = C; \
    } \
} while(0)
#define qpop(Q) do { \
    if(!qempty(Q)){ \
        --Q->size; \
        ++Q->front; \
        if(Q->front == Q->capacity) \
            Q->front = 0; \
    } \
} while(0)
#define qpeek(Q) Q->clients[Q->front]

/* waits for aclient in our queue. once available, it is popped off of the queue & returned. */
aclient_t queue_get(void){
    pthread_mutex_lock(&mutex);

wait_client:
    /* wait for a client. */
    while(qempty(q))
        if(pthread_cond_wait(&cond, &mutex) != 0)
            perror("Cond Wait Error");

    /* got available client in queue. pop it from the queue. */
    aclient_t ca = qpeek(q);
    qpop(q);
    if(src_got(ca)) // if it seems like we already have the file, go next in queue.
        goto wait_client;
    pthread_mutex_unlock(&mutex);
    return ca;
}

void queue_add(aclient_t ca){
    pthread_mutex_lock(&mutex); // lock queue
    /* push new client onto queue. */
    qpush(q, ca);
    pthread_mutex_unlock(&mutex); // unlock queue
    /* inform threads there is a new/available client. */
    pthread_cond_signal(&cond);
}