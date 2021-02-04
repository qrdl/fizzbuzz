#include <stdio.h>
#include <string.h>

#include <immintrin.h>

#define LIMIT 1000000000
// max size of buffer for 15 numbers (except 1B)
#define CHUNK_SIZE  119

static void print(int start);

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

// don't use itoa() because it is non-standard and more generic
static char *myitoa(int number, char *cur) {
    do {
        *--cur = number % 10 + '0';
        number /= 10;
    } while (number != 0);
    return cur;
}

static void myitoa_diff(int number, char *cur, int diff_len) {
    char *end = cur - diff_len;
    do {
        *--cur = number % 10 + '0';
        number /= 10;
    } while (cur > end);
}

static char *myitoa14(int number, char *cur, char *old) {
    *--cur = number % 10 + '0';
    number /= 10;
    *--cur = number % 10 + '0';
    for (;;) {
        number /= 10;
        if (number == 0) {
            return cur;
        }
        char digit = number % 10 + '0';
        if (*--cur == digit) {
            return old;
        }
        *cur = digit;
    }
}

static void print(int num) {
    static char wrkbuf[CHUNK_SIZE] = {
        [CHUNK_SIZE - 10] = '\n', 'F', 'i', 'z', 'z', 'B', 'u', 'z', 'z', '\n'
    };
    static char *pos[8];    // past the end positions of numbers within the buffer
    static int buf_len;     // output length
    static int digit_num;   // number of digits within the number

    // always output num + 13 to check number of digits
    char *cur = myitoa14(num + 13, wrkbuf + CHUNK_SIZE - 10, pos[7]);    // 14

    if (digit_num != wrkbuf + CHUNK_SIZE - 10 - cur) {
        // there are more digits in the number - create buffer from the scratch
        pos[7] = cur;
        *--cur = '\n';                      // 13
        cur = myitoa(num + 12, cur) - 6;
        memcpy(cur, "\nFizz\n", 6);         // 12
        pos[6] = cur;                       // 11
        cur = myitoa(num + 10, cur) - 11;
        memcpy(cur, "\nFizz\nBuzz\n", 11);  // 9, 10
        pos[5] = cur;                       // 8
        cur = myitoa(num + 7, cur) - 1;
        *cur = '\n';                        // 7
        pos[4] = cur;
        cur = myitoa(num + 6, cur) - 11;
        memcpy(cur, "\nBuzz\nFizz\n", 11);  // 5, 6
        pos[3] = cur;                       // 4
        cur = myitoa(num + 3, cur) - 6;
        memcpy(cur, "\nFizz\n", 6);         // 3
        pos[2] = cur;                       // 2
        cur = myitoa(num + 1, cur) - 1;
        *cur = '\n';                        // 1
        pos[1] = cur;
        cur = myitoa(num + 0, cur);
        pos[0] = cur;
        digit_num = pos[1] - cur;
        buf_len = wrkbuf + CHUNK_SIZE - cur;
    } else {
        // find out how many digits actually changed - use SSE2 instruction for comparing 8-byte buffers
        unsigned int diff = ~_mm_movemask_epi8(_mm_cmpeq_epi8(
                                               _mm_loadl_epi64((__m128i const *)pos[0]),
                                               _mm_loadl_epi64((__m128i const *)cur)));
        // lower zero bits indicate unchanged bytes
        unsigned int diff_len = digit_num - _tzcnt_u32(diff);   // number of changed digits

        myitoa_diff(num + 12, cur - 1, diff_len);   // 13
        myitoa_diff(num + 10, pos[6] , diff_len);   // 11
        myitoa_diff(num +  7, pos[5] , diff_len);   // 8
        myitoa_diff(num +  6, pos[4] , diff_len);   // 7
        myitoa_diff(num +  3, pos[3] , diff_len);   // 4
        myitoa_diff(num +  1, pos[2] , diff_len);   // 2
        myitoa_diff(num +  0, pos[1] , diff_len);   // 1
    }

    fwrite(pos[0], buf_len, 1, stdout);
}
