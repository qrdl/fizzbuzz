#include <stdio.h>

#define LIMIT 1000000000

int main(void) {
    for (int i = 1; i <= LIMIT; i++) {
        if (0 == i % 3) {
            printf("Fizz");
        }
        if (0 == i % 5) {
            printf("Buzz");
        }
        if (i % 3 && i % 5) {
            printf("%d", i);
        }
        printf("\n");
    }

    return 0;
}
