#include <stdio.h>
#include <string.h>

#define LIMIT 1000000000
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

// don't use itoa() because it is non-standard and more generic
int myitoa(int number, char *buf) {
    char tmpbuf[12];
    char *cur = tmpbuf+11;
    *cur = '\n';
    int i = 1;  // start from 1 char - newline
    for (; number > 0; i++) {
        cur--;
        int tmp = number % 10;
        number /= 10;
        *cur = tmp + '0';
    }
    memcpy(buf, cur, i);
    return i;
}

#define NUM cur += myitoa(num++, cur)
#define FIZZ do { memcpy(cur, "Fizz\n", 5); cur += 5; num++; } while (0)
#define BUZZ do { memcpy(cur, "Buzz\n", 5); cur += 5; num++; } while (0)
#define FIZZBUZZ do { memcpy(cur, "FizzBuzz\n", 9); cur += 9; } while (0)

void print(int num) {
    static char wrkbuf[CHUNK_SIZE];

    char *cur = wrkbuf;
    NUM;
    NUM;
    FIZZ;
    NUM;
    BUZZ;
    FIZZ;
    NUM;
    NUM;
    FIZZ;
    BUZZ;
    NUM;
    FIZZ;
    NUM;
    NUM;
    FIZZBUZZ;
    fwrite(wrkbuf, cur - wrkbuf, 1, stdout);
}
