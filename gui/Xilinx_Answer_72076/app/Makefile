CFLAGS += -O3 -Os -I $(PWD)
LFLAGS += -lpthread

SYNC_TEST_OBJS = simple-test.o
PIO_TEST_OBJS = pio-test.o

all: SYNC_TEST PIO_TEST

SYNC_TEST:	$(SYNC_TEST_OBJS) 
		$(CC) $(CFLAGS) $(SYNC_TEST_OBJS) $(LFLAGS) -o simple_test

PIO_TEST:	$(PIO_TEST_OBJS) 
		$(CC) $(CFLAGS) $(PIO_TEST_OBJS) $(LFLAGS) -o pio_test

simple-test-test.o:	simple-test.c
		$(CC) -c $(CFLAGS) -I . simple-test.c -o simple-test.o

pio-test.o:	pio-test.c
		$(CC) -c $(CFLAGS) -I . pio-test.c -o pio-test.o


clean:
	rm -f $(SYNC_TEST_OBJS) simple_test $(PIO_TEST_OBJS) pio_test
