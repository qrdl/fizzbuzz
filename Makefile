CFLAGS := -O3 -march=native -pthread

.PHONY : all clean

BINS = supernaive naive unrolled customprint customprint2 reusebuffer reusebuffer2 reduceprint reduceandreuse multithreaded

all: $(BINS)

clean:
	rm -f $(BINS) *.o

