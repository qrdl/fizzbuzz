CFLAGS := -O3 -march=native -pthread

.PHONY : all clean

BINS = supernaive naive unrolled customprint reusebuffer multithreaded

all: $(BINS)

clean:
	rm -f $(BINS) *.o

