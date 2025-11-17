void updtitlebar(WINDOW *t, const u_short master_port, const queue_t *aq, const u_int active_threads){
    werase(t);
#ifdef __x86_64__
    wprintw(t, "Listening on port: %hu  |  Current queue size:  %lu, (%.2f%% occupied)  |  Active threads: %u", master_port, aq->size, (aq->size/aq->capacity)*100, active_threads);
#else
    wprintw(t, "Listening on port: %hu  |  Current queue size:  %u, (%.2f%% occupied)  |  Active threads: %u", master_port, aq->size, (aq->size/aq->capacity)*100, active_threads);
#endif
    wrefresh(t);
}

void initwindows(void){
    /* obligatory curses init */
    initscr();
    cbreak(); // disable line buffering for input.
    noecho(); // do not echo input.
    /* all notifications are sent to this window. */
    mainwindow = newwin(LINES-1, COLS, 2, 0);
}