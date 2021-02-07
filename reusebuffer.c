#include <stdio.h>
#include <string.h>

#include <immintrin.h>

#define LIMIT 1000000000
// use 16-byte buffer for SSE2 128-bit vector instructions
#define NUMBERSIZE 16
// max size of buffer for 15 numbers (except 1B)
#define CHUNK_SIZE  119

void print(int start);

int main(void) {
    int i;
    for (i = 1; i < LIMIT - 15; i += 15) {
        print(i);
    }
    while (i <= LIMIT) {
        if (i % 3 == 0) {
            printf("Fizz\n");
        } else if (i % 5 == 0) {
            printf("Buzz\n");
        } else {
            printf("%d\n", i);
        }
        i++;
    }

    return 0;
}

// return number of decimal digits within the number, write decimal number to supplied buffer, right-aligned
int myitoa(int number, char buf[NUMBERSIZE]) {
    char *cur = buf+NUMBERSIZE;
    int i = 0;
    for (; number > 0; i++) {
        cur--;
        int tmp = number % 10;
        number /= 10;
        *cur = tmp + '0';
    }
    return i;
}

#define NUM do { int tmp = myitoa(num++, number); memcpy(cur, number+NUMBERSIZE-tmp, tmp+1); cur += tmp+1; } while (0)
#define FIZZ do { memcpy(cur, "Fizz\n", 5); cur += 5; num++; } while (0)
#define BUZZ do { memcpy(cur, "Buzz\n", 5); cur += 5; num++; } while (0)
#define FIZZBUZZ do { memcpy(cur, "FizzBuzz\n", 9); cur += 9; } while (0)

#define PROCNUM(I) do { \
        int cur = num+I-1; \
        for (int i = pos[I-1] + digit_num - 1; i >= (int)(pos[I-1] + digit_num - diff_pos); i--) { \
            wrkbuf[i] = (cur % 10) + '0'; \
            cur /= 10; \
        } \
} while (0)

void print(int num) {
    static int pos[15];     // positions of numbers withing the buffer
    static char wrkbuf[CHUNK_SIZE];
    static int buf_len;
    static int digit_num = 0;   // number of digits within the number

    char last_number[NUMBERSIZE] = {0};
    int digit_count = myitoa(num+14, last_number);

    // numbers add extra digit only 8 times during the whole execution, so this branch is unlikely
    if (__builtin_expect(digit_num != digit_count, 0)) {
        // there are more digits in the number - create buffer from the scratch
        char number[NUMBERSIZE+1];  // one extra for newline
        number[NUMBERSIZE] = '\n';
        char *cur = wrkbuf;
        pos[0] = 0;
        NUM;
        if (cur - wrkbuf - 1 == digit_count) {
            digit_num = digit_count;
        }
        pos[1] = cur - wrkbuf;
        NUM;
        pos[2] = cur - wrkbuf;
        FIZZ;
        pos[3] = cur - wrkbuf;
        NUM;
        pos[4] = cur - wrkbuf;
        BUZZ;
        pos[5] = cur - wrkbuf;
        FIZZ;
        pos[6] = cur - wrkbuf;
        NUM;
        pos[7] = cur - wrkbuf;
        NUM;
        pos[8] = cur - wrkbuf;
        FIZZ;
        pos[9] = cur - wrkbuf;
        BUZZ;
        pos[10] = cur - wrkbuf;
        NUM;
        pos[11] = cur - wrkbuf;
        FIZZ;
        pos[12] = cur - wrkbuf;
        NUM;
        pos[13] = cur - wrkbuf;
        NUM;
        pos[14] = cur - wrkbuf;
        FIZZBUZZ;
        buf_len = cur - wrkbuf;
    } else {
        // for comparison use first number from current working buffer
        char prev_first_number[NUMBERSIZE] = {0};
        int num_width = pos[1] - 1;
        memcpy(prev_first_number + NUMBERSIZE - num_width, wrkbuf, num_width);

        // find out how many digits actually changed - use SSE2 instruction for comparing 16-byte buffers
        unsigned int diff = 0xFFFF & ~_mm_movemask_epi8(_mm_cmpeq_epi8(
                                                        _mm_load_si128((__m128i const *)prev_first_number),
                                                        _mm_load_si128((__m128i const *)last_number)));
        // lower zero bits indicate unchanged bytes
        unsigned int diff_pos = 16 - _tzcnt_u32(diff);   // number of changed digits

        PROCNUM(1);
        PROCNUM(2);
        PROCNUM(4);
        PROCNUM(7);
        PROCNUM(8);
        PROCNUM(11);
        PROCNUM(13);
        PROCNUM(14);
    }

    fwrite(wrkbuf, buf_len, 1, stdout);
}
