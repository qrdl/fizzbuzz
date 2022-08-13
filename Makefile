CFLAGS := -O3 -march=native -pthread

.PHONY : all clean

BINS = supernaive naive unrolled customprint customprint2 reusebuffer reusebuffer2 multithreaded multithreaded2

multithreaded2: multithreaded2.c lookup.h

all: $(BINS)

clean:
	rm -f $(BINS) *.o

