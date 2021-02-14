#include <stdio.h>
#include <string.h>

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

static char *memcpyr(char *dst, char *src, int len) {
    return memcpy(dst - len, src, (size_t)len);
}

static void print(const int num) {
    static char wrkbuf[CHUNK_SIZE] = {
        [CHUNK_SIZE - 11] = '9', '\n', 'F', 'i', 'z', 'z', 'B', 'u', 'z', 'z', '\n'
    };

    const int num10 = num / 10;
    char *cur = wrkbuf + CHUNK_SIZE - 11;

    char *nums = myitoa(num10 + 2, cur);        // 29
    int numlen = (int)(cur - nums);
    cur = memcpyr(nums, "8\n", 2);              // 28
    cur = memcpyr(cur, nums, numlen);
    cur = memcpyr(cur, "6\nFizz\n", 7);         // 27
    cur = memcpyr(cur, nums, numlen);           // 26
    cur = memcpyr(cur, "3\nFizz\nBuzz\n", 12);  // 24, 25
    cur = memcpyr(cur, nums, numlen);           // 23
    cur = memcpyr(cur, "2\n", 2);               // 22
    cur = memcpyr(cur, nums, numlen);
    cur = memcpyr(cur, "9\nBuzz\nFizz\n", 12);  // 20, 21

    nums = myitoa(num10 + 1, cur);              // 19
    numlen = (int)(cur - nums);
    cur = memcpyr(nums, "7\nFizz\n", 7);        // 18
    cur = memcpyr(cur, nums, numlen);           // 17
    cur = memcpyr(cur, "6\n", 2);               // 16
    cur = memcpyr(cur, nums, numlen);
    cur = memcpyr(cur, "4\nFizzBuzz\n", 11);    // 15
    cur = memcpyr(cur, nums, numlen);           // 14
    cur = memcpyr(cur, "3\n", 2);               // 13
    cur = memcpyr(cur, nums, numlen);
    cur = memcpyr(cur, "1\nFizz\n", 7);         // 12
    cur = memcpyr(cur, nums, numlen);           // 11
    cur = memcpyr(cur, "8\nFizz\nBuzz\n", 12);  // 9, 10

    nums = myitoa(num10 + 0, cur);              // 8
    numlen = (int)(cur - nums);
    cur = memcpyr(nums, "7\n", 2);              // 7
    cur = memcpyr(cur, nums, numlen);
    cur = memcpyr(cur, "4\nBuzz\nFizz\n", 12);  // 5, 6
    cur = memcpyr(cur, nums, numlen);           // 4
    cur = memcpyr(cur, "2\nFizz\n", 7);         // 3
    cur = memcpyr(cur, nums, numlen);           // 2
    cur = memcpyr(cur, "1\n", 2);               // 1
    cur = memcpyr(cur, nums, numlen);

    fwrite(cur, (size_t)(wrkbuf + CHUNK_SIZE - cur), 1, stdout);
}
