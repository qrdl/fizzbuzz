# fizzbuzz

Optimisation of classic [FizzBuzz](http://wiki.c2.com/?FizzBuzzTest) test.

### supernaive
The least efficient implementation, with 3 *if*s and two *printf*s per number. It is so inefficient that I don't even use it for comparison.

### naive
A little bit optimised version, with 2 *if*s (not counting the loop condition) and 1 *printf* per number.

### unrolled
Single loop iteration for 15 numbers, it means for 15 numbers just one branch (vs 45 branches for naive) and 1 *printf* (vs 15 *printf*s for naive).

### customprint
Generic *printf* replaced with custom print routine, tailored for this particular task.

### customprint2
Like *customprint*, but buffer is filled in reverse, and adjacent *Fizz* and *Buzz* words are merged into single *memcpy*, as a result number of *memcpy* calls per iteration goes down from 15 to just 4. Courtesy of [kariya-mitsuru](https://github.com/kariya-mitsuru).

### reusebuf
Reuse buffer from previous iteration, update only the changed characters. Use x86_64 vector instructions for comparing buffers.

### reusebuf2
Reuse buffer from previous iteration, update only the changed characters. Use x86_64 vector instructions for comparing buffers. Fill buffer in reverse to reduce number of *memcpy* calls. Courtesy of [kariya-mitsuru](https://github.com/kariya-mitsuru).

### multithreaded
Use worker threads to process the sets of numbers in parallel.

## Comparison

All tests are performed on Dell Latitude 7480 with Core i7-7600U (2 cores with hyperthreading) and 16 Gb RAM, OpenSUSE Leap 15.1, kernel 4.12.14, gcc 7.5.0.

Output redirected to /dev/null. Multithreaded implementation uses 4 worker threads, with load of 3M number per thread.

Implementation | Time (sec.millisec) | Relative to naive | Relative to previous
-|-|-|-
supernaive | 49.661 | 0.80 |
naive | 39.650 | 1 | 1.25
unrolled | 20.151 | 1.97 | 1.97
customprint | 8.771 | 4.52 | 2.30
customprint2 | 6.695 | 5.92 | 1.31
reusebuf | 4.490 | 8.83 | 1.49
reusebuf2 | 2.818 | 14.07 | 1.59
multithreaded | 1.748 | 22.68 | 1.61
