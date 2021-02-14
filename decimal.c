#include <stdio.h>
#include <string.h>

#define LIMIT 1000000000
char *FIZZ = "Fizz";
char *BUZZ = "Buzz";

struct Segment {
    char buffer[4096]; // 100 numbers
    int buffer_length;
    int positions[100]; // offsets in buffer1 of third digit of all numbers
    int positions_length; // number of items in positions
};

int get_number_length(int number) {
    int number_length = 1;
    while (number >= 10) {
        number /= 10;
        number_length++;
    }
    return number_length;
}

char* print_number(int number, int numberLength, char* buf) {
    for(int i = 0; i < numberLength; i++) {
        int digit = number % 10;
        buf[numberLength - i - 1] = (digit + '0');
        number /= 10;
    }
    buf[numberLength] = '\n';
    return buf + numberLength + 1;
}

char* print_fizz(char* buf) {
    *(int*)buf = *(int*)FIZZ;
    buf[4] = '\n';
    return buf + 5;
}

char *print_buzz(char *buf) {
    *(int *)buf = *(int *)BUZZ;
    buf[4] = '\n';
    return buf + 5;
}

char *print_fizz_buzz(char *buf) {
    *(int *)buf = *(int *)FIZZ;
    *(int *)(buf + 4) = *(int *)BUZZ;
    buf[8] = '\n';
    return buf + 9;
}

char *print(int num, int num_length, char *ptr) {
    if (num_length == -1)
        num_length = get_number_length(num);
    if (num % 15 == 0)
        ptr = print_fizz_buzz(ptr);
    else if (num % 3 == 0)
        ptr = print_fizz(ptr);
    else if (num % 5 == 0)
        ptr = print_buzz(ptr);
    else
        ptr = print_number(num, num_length, ptr);
    return ptr;
}

char *print_first_segment(char *ptr) {
    for(int num = 1; num < 100; num++)
        ptr = print(num, -1, ptr);
    return ptr;
}

char *print_last_number(char *ptr) {
    return print(LIMIT, -1, ptr);
}

void print_initial_segment(int first_number, struct Segment* segment) {
    int digits = get_number_length(first_number);
    segment->positions_length = 0;
    char *ptr = segment->buffer;
    for (int i = first_number; i < first_number + 100; i++) {
        ptr = print(i, digits, ptr);
        if (i % 3 != 0 && i % 5 != 0)
            segment->positions[segment->positions_length++] = ptr - segment->buffer - 4;
    }
    segment->buffer_length = ptr - segment->buffer;
}

void print_initial_segments(int segment, struct Segment* s1, struct Segment* s2, struct Segment* s3) {
    print_initial_segment(segment, s1);
    print_initial_segment(segment + 100, s2);
    print_initial_segment(segment + 200, s3);
}

void next(struct Segment* s) {
    char c = s->buffer[s->positions[0]];
    char nextC = c + 3;
    if(c < '7') {
        for (int i = 0; i < s->positions_length; i++)
            s->buffer[s->positions[i]] = nextC;
    } else if (s->buffer[s->positions[0] - 1] < '9') {
        nextC -= 10;
        int nextD = s->buffer[s->positions[0] - 1] + 1;
        for (int i = 0; i < s->positions_length; i++) {
            s->buffer[s->positions[i]] = nextC;
            s->buffer[s->positions[i] - 1] = nextD;
        }
    } else {
        nextC -= 10;
        for (int i = 0; i < s->positions_length; i++) {
            int pos = s->positions[i];
            s->buffer[pos--] = nextC;
            while(s->buffer[pos] == '9')
                s->buffer[pos--] = '0';
            s->buffer[pos]++;
        }
    }
}

int main(void) {
    char* ptr;
    struct Segment s1, s2, s3;

    ptr = print_first_segment(s1.buffer);
    fwrite(s1.buffer, 1, ptr - s1.buffer, stdout);

    for (int segment = 100; segment < LIMIT; segment *= 10) {
        print_initial_segments(segment, &s1, &s2, &s3);
        fwrite(s1.buffer, 1, s1.buffer_length, stdout);
        fwrite(s2.buffer, 1, s2.buffer_length, stdout);
        fwrite(s3.buffer, 1, s3.buffer_length, stdout);
        int repeat_count = 3 * (segment / 100);
        for(int i = 1; i < repeat_count; i++) {
            next(&s1);
            next(&s2);
            next(&s3);
            fwrite(s1.buffer, 1, s1.buffer_length, stdout);
            fwrite(s2.buffer, 1, s2.buffer_length, stdout);
            fwrite(s3.buffer, 1, s3.buffer_length, stdout);
        }
    }

    ptr = print_last_number(s1.buffer);
    fwrite(s1.buffer, 1, ptr - s1.buffer, stdout);

    return 0;
}

