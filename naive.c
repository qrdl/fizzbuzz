#include <stdio.h>

#define LIMIT 1000000000

int main(void) {
    for (int i = 1; i <= LIMIT; i++) {
        if (0 == i % 3) {
            if (0 == i % 5) {
                printf("FizzBuzz\n");
            } else {
                printf("Fizz\n");
            }
        } else if (0 == i % 5) {
            printf("Buzz\n");
        } else {
            printf("%d\n", i);
        }
    }

    return 0;
}
