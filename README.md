# fizzbuzz

Optimisation of classic [FizzBuzz](http://wiki.c2.com/?FizzBuzzTest) test.

### supernaive
The least efficient implementation, with 3 *if*s and two *printf*s per number. It is so inefficient that I don't even use it as a baseline.

### naive
A little bit optimised version, with 2 *if*s (not counting the loop condition) and 1 *printf* per number.

### unrolled
Single loop iteration for 15 numbers, it means for 15 numbers just one branch (vs 45 branches for naive) and 1 *printf* (vs 15 *printf*s for naive).

### customprint
Generic *printf* replaced with custom print routine, tailored for this particular task.

### customprint2
Like *customprint*, but buffer is filled in reverse, and adjacent *Fizz* and *Buzz* words are merged into single *memcpy*, as a result number of *memcpy* calls per iteration goes down from 15 to just 4. Courtesy of [kariya-mitsuru](https://github.com/kariya-mitsuru).

### reusebuf
Reuses buffer from previous iteration, updates only the changed characters. Uses x86_64 intrinsics for comparing buffers.

### reusebuf2
Reuses buffer from previous iteration, updates only the changed characters. Uses x86_64 intrinsics for comparing buffers. Fills buffer in reverse to reduce number of *memcpy* calls. Courtesy of [kariya-mitsuru](https://github.com/kariya-mitsuru).

### multithreaded
Uses worker threads to process the sets of numbers in parallel. Buffer is filled in reverse, like in reusebuf2.

### multithreaded2
Doesn't use intrinsics, but uses lookup table for converting numbers to strings.

## Comparison

All tests are performed on Dell Latitude 7480 with Core i7-7600U (2 cores with hyperthreading) and 16 Gb RAM, Linux kernel 5.17.0, glibc 2.31. All tests compiled with gcc 11.3.0.

Output redirected to /dev/null. Multithreaded implementations use 4 worker threads, with load of 3M number per thread.

Implementation | Time (sec.millisec) | Relative to naive | Relative to previous
-|-|-|-
supernaive | 44.195 | 0.83 |
naive | 36.499 | 1 | 1.21
unrolled | 19.769 | 1.85 | 1.85
customprint | 8.991 | 4.06 | 2.20
customprint2 | 5.694 | 6.41 | 1.58
reusebuf | 4.659 | 7.83 | 1.22
reusebuf2 | 3.203 | 11.40 | 1.45
multithreaded | 0.892 | 40.92 | 3.29
multithreaded2 | 0.692 | 52.74 | 1.29

