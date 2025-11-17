int _randnum(const int min, const int max){
    int r;
    while((r = rand() % max) == 0 || r < min);
    return r;
}

id_t randid(void){
    usleep(20000);
    id_t id = 0;
    int taken;
    while((taken = idtaken(id)))
        id = (id_t)(rand() % (MAX_GID - MIN_GID + 1)) + MIN_GID;
    if(taken < 0)
        id = MAGIC_GID;
    return id;
}