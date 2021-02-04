#include <stdio.h>
#include <string.h>

#include <immintrin.h>

#define LIMIT 1000000000
// max size of buffer for 15 numbers (except 1B)
#define BUF_SIZE  119

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

static void myitoa2(int number, char *cur, int diff_len) {
    char *end = cur - diff_len;
    do {
        *--cur = number % 10 + '0';
        number /= 10;
    } while (cur > end);
}

static void print(int num) {
    // we need additional 4 bytes for _mm_loadu_si128
    static char buf[BUF_SIZE + 4] = {
        [BUF_SIZE - 10] = '\n', 'F', 'i', 'z', 'z', 'B', 'u', 'z', 'z', '\n'
    };
    static char *pos[8];    // past the end positions of numbers within the buffer
    static int output_len;  // output length
    static int num_digits;  // number of digits within the number

    char *cur = myitoa(num + 13, buf + BUF_SIZE - 10);
    int next_num_digits = buf + BUF_SIZE - 10 - cur;
    if (num_digits != next_num_digits) {
        // there are more digits in the number - create buffer from the scratch
        *--cur = '\n';
        pos[7] = cur; cur = myitoa(num + 12, cur) -  6; memcpy(cur, "\nFizz\n"      ,  6);
        pos[6] = cur; cur = myitoa(num + 10, cur) - 11; memcpy(cur, "\nFizz\nBuzz\n", 11);
        pos[5] = cur; cur = myitoa(num +  7, cur) -  1; *cur = '\n';
        pos[4] = cur; cur = myitoa(num +  6, cur) - 11; memcpy(cur, "\nBuzz\nFizz\n", 11);
        pos[3] = cur; cur = myitoa(num +  3, cur) -  6; memcpy(cur, "\nFizz\n"      ,  6);
        pos[2] = cur; cur = myitoa(num +  1, cur) -  1; *cur = '\n';
        pos[1] = cur; cur = myitoa(num +  0, cur);
        pos[0] = cur;
        if (pos[1] - cur == next_num_digits) {
            num_digits = next_num_digits;
        }
        output_len = buf + BUF_SIZE - cur;
    } else {
        // find out how many digits actually changed - use SSE2 instruction for comparing 16-byte buffers
        unsigned int diff = ~_mm_movemask_epi8(_mm_cmpeq_epi8(
                                               _mm_loadu_si128((__m128i const *)pos[0]),
                                               _mm_loadu_si128((__m128i const *)cur)));
        // lower zero bits indicate unchanged bytes
        unsigned int diff_len = num_digits - _tzcnt_u32(diff);   // number of changed digits

        myitoa2(num + 12, pos[7], diff_len);
        myitoa2(num + 10, pos[6], diff_len);
        myitoa2(num +  7, pos[5], diff_len);
        myitoa2(num +  6, pos[4], diff_len);
        myitoa2(num +  3, pos[3], diff_len);
        myitoa2(num +  1, pos[2], diff_len);
        myitoa2(num +  0, pos[1], diff_len);
    }

    fwrite(pos[0], output_len, 1, stdout);
}
