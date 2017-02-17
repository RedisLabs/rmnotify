CC = gcc
CFLAGS = -L. -levent -levent_pthreads -lnotify -I.

LDIR = ../deps/hiredis
OBJ = keyspacenote.o

LIBNAME = libnotify
STLIBSUFFIX=a
STLIBNAME=$(LIBNAME).$(STLIBSUFFIX)

all: $(STLIBNAME) demo

$(OBJ):
	$(CC) -c keyspacenote.c -o $(OBJ)

$(STLIBNAME): $(OBJ)
	ar rcs $(STLIBNAME) $(OBJ)

static: $(STLIBNAME)

demo: static
	$(CC) demo.c $(LDIR)/libhiredis.a -o demo $(CFLAGS)

clean:
	rm -f $(STLIBNAME) $(OBJ) demo