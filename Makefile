
CC = gcc
CFLAGS = -g -Wall 
OS = $(shell uname -s)
PROC = $(shell uname -p)
EXEC_SUFFIX=$(OS)-$(PROC)

ifeq ("$(OS)", "SunOS")
	OSLIB=-L/opt/csw/lib -R/opt/csw/lib -lsocket -lnsl
	OSINC=-I/opt/csw/include
	OSDEF=-DSOLARIS
else
ifeq ("$(OS)", "Darwin")
	OSLIB=
	OSINC=
	OSDEF=-DDARWIN
else
	OSLIB=
	OSINC=
	OSDEF=-DLINUX
endif
endif

all:  keyboard-$(EXEC_SUFFIX)

keyboard-$(EXEC_SUFFIX): keyboard.c
	$(CC) $(CFLAGS) $(OSINC) $(OSLIB) $(OSDEF) -o i_am_keyboard keyboard.c -lpcap checksum.c smartalloc.c 

clean:
	-rm -rf keyboard-* keyboard-*.dSYM i_am_keyboard
