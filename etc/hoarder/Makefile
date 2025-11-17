WARNING_FLAGS+=-Wall -Wextra -Werror
OPTION_FLAGS+=-fomit-frame-pointer -Os -g0
LINK_FLAGS+=-lpthread -lcurses
OUTNAME+=./hoarder

all: clean hoarder

hoarder: main.c queue.h client.h dirs.h
	$(CC) $(LINK_FLAGS) $(OPTION_FLAGS) $(WARNING_FLAGS) -o $(OUTNAME) main.c -lpthread
	strip $(OUTNAME)

clean:
	rm -f $(OUTNAME)