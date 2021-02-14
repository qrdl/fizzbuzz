#include <stdio.h>
#include <string.h>

#include <immintrin.h>

#define LIMIT 1000000000
// max size of buffer for 30 numbers (except 1B)
#define CHUNK_SIZE  (10 * 16 + 4 * (6 + 10) + 30)

static void print(int start);

int main(void) {
    int i;
    for (i = 1; i <= LIMIT - 29; i += 30) {
        print(i);
    }
    for (; i <= LIMIT; ++i) {
        if (i % 3 == 0) {
            puts("Fizz");
        } else if (i % 5 == 0) {
            puts("Buzz");
        } else {
            printf("%d\n", i);
        }
    }
}

// don't use itoa() because it is non-standard and more generic
static char *myitoa(int number, char *cur) {
    for (; number != 0; number /= 10) {
        *--cur = (char)(number % 10 + '0');
    }
    return cur;
}

static void myitoa_diff(int number, char *cur, char *end) {
    do {
        *--cur = (char)(number % 10 + '0');
        number /= 10;
    } while (cur > end);
}

static char *myitoa29(int number, char *cur, char *old) {
    *--cur = (char)(number % 10 + '0');
    for (;;) {
        number /= 10;
        if (number == 0) {
            return cur;
        }
        char digit = (char)(number % 10 + '0');
        if (*--cur == digit) {
            return old;
        }
        *cur = digit;
    }
}

static char *memcpyr(char *dst, char *src, int len) {
    return memcpy(dst - len, src, (size_t)len);
}

static void print(const int num) {
    static char wrkbuf[CHUNK_SIZE] = {
        [CHUNK_SIZE - 11] = '9', '\n', 'F', 'i', 'z', 'z', 'B', 'u', 'z', 'z', '\n'
    };
    static char *pos[16];   // past the end positions of numbers within the buffer
    static int buf_len;     // output length
    static int digit_num;   // number of digits within the number

    const int num10 = num / 10;

    // always output num + 28 to check number of digits
    char *cur = wrkbuf + CHUNK_SIZE - 11;
    char *nums = myitoa29(num10 + 2, cur, pos[15] + 2);     // 29

    if (digit_num != cur - nums) {
        // there are more digits in the number - create buffer from the scratch

        digit_num = (int)(cur - nums);
        pos[15] = memcpyr(nums, "8\n", 2);                  // 28
        cur = memcpyr(pos[15], nums, digit_num);
        pos[14] = memcpyr(cur, "6\nFizz\n", 7);             // 27
        cur = memcpyr(pos[14], nums, digit_num);            // 26
        pos[13] = memcpyr(cur, "3\nFizz\nBuzz\n", 12);      // 24, 25
        cur = memcpyr(pos[13], nums, digit_num);            // 23
        pos[12] = memcpyr(cur, "2\n", 2);                   // 22
        cur = memcpyr(pos[12], nums, digit_num);
        pos[11]= memcpyr(cur, "9\nBuzz\nFizz\n", 12);       // 20, 21

        nums = myitoa(num10 + 1, pos[11]);                  // 19
        digit_num = (int)(pos[11] - nums);
        pos[10] = memcpyr(nums, "7\nFizz\n", 7);            // 18
        cur = memcpyr(pos[10], nums, digit_num);            // 17
        pos[ 9] = memcpyr(cur, "6\n", 2);                   // 16
        cur = memcpyr(pos[ 9], nums, digit_num);
        pos[ 8] = memcpyr(cur, "4\nFizzBuzz\n", 11);        // 15
        cur = memcpyr(pos[ 8], nums, digit_num);            // 14
        pos[ 7] = memcpyr(cur, "3\n", 2);                   // 13
        cur = memcpyr(pos[ 7], nums, digit_num);
        pos[ 6] = memcpyr(cur, "1\nFizz\n", 7);             // 12
        cur = memcpyr(pos[ 6], nums, digit_num);            // 11
        pos[ 5] = memcpyr(cur, "8\nFizz\nBuzz\n", 12);      // 9, 10

        nums = myitoa(num10 + 0, pos[ 5]);                  // 8
        digit_num = (int)(pos[ 5] - nums);
        pos[ 4] = memcpyr(nums, "7\n", 2);                  // 7
        cur = memcpyr(pos[ 4], nums, digit_num);
        pos[ 3] = memcpyr(cur, "4\nBuzz\nFizz\n", 12);      // 5, 6
        cur = memcpyr(pos[ 3], nums, digit_num);
        pos[ 2] = memcpyr(cur, "2\nFizz\n", 7);             // 3
        cur = memcpyr(pos[ 2], nums, digit_num);            // 2
        pos[ 1] = memcpyr(cur, "1\n", 2);                   // 1
        pos[ 0] = memcpyr(pos[ 1], nums, digit_num);

        buf_len = (int)(wrkbuf + CHUNK_SIZE - pos[0]);
    } else {
        // find out how many digits actually changed - use SSE2 instruction for comparing 8-byte buffers
        int diff = ~_mm_movemask_epi8(_mm_cmpeq_epi8(
                                      _mm_loadl_epi64((__m128i const *)pos[0]),
                                      _mm_loadl_epi64((__m128i const *)nums)));
        // lower zero bits indicate unchanged bytes
        int diff_len = digit_num - (int)_tzcnt_u32((unsigned)diff);   // number of changed digits

        nums = cur - diff_len;
        memcpyr(pos[15], nums, diff_len);       // 28
        memcpyr(pos[14], nums, diff_len);       // 26
        memcpyr(pos[13], nums, diff_len);       // 23
        memcpyr(pos[12], nums, diff_len);       // 22

        nums = pos[11] - diff_len;
        myitoa_diff(num10 + 1, pos[11], nums);  // 19
        memcpyr(pos[10], nums, diff_len);       // 17
        memcpyr(pos[ 9], nums, diff_len);       // 16
        memcpyr(pos[ 8], nums, diff_len);       // 14
        memcpyr(pos[ 7], nums, diff_len);       // 13
        memcpyr(pos[ 6], nums, diff_len);       // 11

        nums = pos[ 5] - diff_len;
        myitoa_diff(num10 + 0, pos[ 5], nums);  // 8
        memcpyr(pos[ 4], nums, diff_len);       // 7
        memcpyr(pos[ 3], nums, diff_len);       // 4
        memcpyr(pos[ 2], nums, diff_len);       // 2
        memcpyr(pos[ 1], nums, diff_len);       // 1
    }

    fwrite(pos[0], (size_t)buf_len, 1, stdout);
}
