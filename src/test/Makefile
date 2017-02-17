CC = gcc
CFLAGS = -L../ -levent -levent_pthreads -lnotify

OBJ = test_notifications.o

test: $(OBJ)
	$(CC) test_notifications.c ../../deps/hiredis/libhiredis.a -o test $(CFLAGS)

clean:
	-rm -f *.o test
