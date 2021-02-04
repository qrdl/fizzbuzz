CFLAGS := -O3 -march=native -pthread

.PHONY : all clean

BINS = supernaive naive unrolled customprint customprint2 reusebuffer multithreaded

all: $(BINS)

clean:
	rm -f $(BINS) *.o

