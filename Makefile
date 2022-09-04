CFLAGS := -O3 -march=native -pthread -fwhole-program

.PHONY : all clean

BINS = supernaive naive unrolled customprint customprint2 reusebuffer reusebuffer2 multithreaded multithreaded2 multithreaded3

all: $(BINS)

clean:
	rm -f $(BINS) *.o

