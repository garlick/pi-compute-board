# install libev-dev

PROGS = canpower cancon cansnoop canping
CANMGRDIR := $(shell pwd)/../libcanmgr
CFLAGS = -I$(CANMGRDIR)
LIBADD = -L$(CANMGRDIR) -lcanmgr -lev

all: $(PROGS)

canpower: canpower.o lxcan.o
	$(CC) -o $@ $^ $(LIBADD)

canping: canping.o lxcan.o monotime.o
	$(CC) -o $@ $^ $(LIBADD)

cancon: cancon.o lxcan.o
	$(CC) -o $@ $^ $(LIBADD)

cansnoop: cansnoop.o lxcan.o monotime.o
	$(CC) -o $@ $^ $(LIBADD)

clean:
	rm -f $(PROGS)
	rm -f *.o
