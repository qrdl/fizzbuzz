#include <stdio.h>
#include <string.h>

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
char *myitoa(int number, char *cur) {
    do {
        *--cur = number % 10 + '0';
        number /= 10;
    } while (number != 0);
    return cur;
}

void print(int num) {
    static char wrkbuf[CHUNK_SIZE] = {
        [CHUNK_SIZE - 10] = '\n', 'F', 'i', 'z', 'z', 'B', 'u', 'z', 'z', '\n'
    };

    char *cur = wrkbuf + CHUNK_SIZE - 10;
    cur = myitoa(num + 13, cur) - 1;    // 14
    *cur = '\n';                        // 13
    cur = myitoa(num + 12, cur) - 6;
    memcpy(cur, "\nFizz\n", 6);         // 12
    cur = myitoa(num + 10, cur) - 11;   // 11
    memcpy(cur, "\nFizz\nBuzz\n", 11);  // 9, 10
    cur = myitoa(num + 7, cur) - 1;     // 8
    *cur = '\n';                        // 7
    cur = myitoa(num + 6, cur) - 11;
    memcpy(cur, "\nBuzz\nFizz\n", 11);  // 5, 6
    cur = myitoa(num + 3, cur) - 6;     // 4
    memcpy(cur, "\nFizz\n", 6);         // 3
    cur = myitoa(num + 1, cur) - 1;     // 2
    *cur = '\n';                        // 1
    cur = myitoa(num + 0, cur);

    fwrite(cur, wrkbuf + CHUNK_SIZE - cur, 1, stdout);
}
