CFLAGS := -O3 -march=native -pthread

.PHONY : all clean

BINS = supernaive naive unrolled customprint reusebuffer reusebuffer2 multithreaded

all: $(BINS)

clean:
	rm -f $(BINS) *.o

