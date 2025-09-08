CC = gcc
CFLAGS = -Wall -Wextra -g

OBJS = storage_mgr.o dberror.o test_assign1_1.o

all: test_assign1_1

test_assign1_1: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

storage_mgr.o: storage_mgr.c storage_mgr.h dberror.h
	$(CC) $(CFLAGS) -c storage_mgr.c

dberror.o: dberror.c dberror.h
	$(CC) $(CFLAGS) -c dberror.c

test_assign1_1.o: test_assign1_1.c storage_mgr.h dberror.h
	$(CC) $(CFLAGS) -c test_assign1_1.c

clean:
	rm -f *.o test_assign1_1 testfile.bin

.PHONY: all clean
