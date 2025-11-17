CFG_PATH+=./config.ini

WARNING_FLAGS+=-Wall -Wextra -Werror -Wno-unused-parameter -Wno-type-limits
OPTION_FLAGS+=-fPIC -fomit-frame-pointer -Os -g0
LINK_FLAGS+=-lc -ldl -lcrypt -lpcap

SONAME+=bdvl.so
SRC+=src
OUTDIR+=build
INC+=$(SRC)/include
CFGH+=$(INC)/config.h
BDVH+=$(INC)/bdv.h
PLATFORM+=$(shell uname -m)

all: setup kit

setup:
	mkdir -p ./$(OUTDIR)
	env python setup.py $(CFG_PATH)

kit: $(SRC)/bdv.c
	$(CC) -std=gnu99 -g $(OPTION_FLAGS) $(WARNING_FLAGS) -I$(SRC) -shared -Wl,--build-id=none $(SRC)/bdv.c $(LINK_FLAGS) -o $(OUTDIR)/$(SONAME).$(PLATFORM)
	-$(CC) -m32 -std=gnu99 -g $(OPTION_FLAGS) $(WARNING_FLAGS) -I$(SRC) -shared -Wl,--build-id=none $(SRC)/bdv.c $(LINK_FLAGS) -o $(OUTDIR)/$(SONAME).i686
	strip $(OUTDIR)/$(SONAME)*

clean:
	rm -rf $(OUTDIR)/$(SONAME)* $(CFGH)
	echo '/* setup.py territory */' > $(BDVH)

cleanall: clean
	rm -rf $(OUTDIR)