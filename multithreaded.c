#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <immintrin.h>

#define LIMIT 1000000000
// use 128 bit number buffers to simplify SSE2 128-bit buffer comparison
#define NUMBERSIZE 16

// 4 worker threads with load of 3M numbers per thread give best result on Core i7-7600U. YMMV
#define THREAD_COUNT    4
// must be multiple of 15
#define NUMS_PER_THREAD 3000000
#define CHUNK_SIZE 119
// per-thread buffer size ~ 22.7 MB (4 threads, 3M numbers load)
#define BUFFER_SIZE (CHUNK_SIZE * NUMS_PER_THREAD / 15 + 1) 

struct thread_data {
    pthread_t   id;
    int         start_num;
    int         count;
    char        *buf;
    int         buflen;
};
struct thread_data thread_pool[THREAD_COUNT];

void *worker(void *arg);
int process_chunk(int num, int *pos, char *wrkbuf, int wrkbuf_size, int *digit_num);

int main(void) {
    int i = 1;
    // start all workers
    for (int j = 0; j < THREAD_COUNT; j++) {
        thread_pool[j].start_num = i;
        thread_pool[j].count = NUMS_PER_THREAD;
        thread_pool[j].buf = malloc(BUFFER_SIZE);
        pthread_create(&thread_pool[j].id, NULL, worker, (void *)&thread_pool[j]);
        i += NUMS_PER_THREAD;
    }
    int active_threads = THREAD_COUNT;
    int max = LIMIT / 15 * 15;  // max number which is multiple of 15
    // wait for workers, print result and start worker again for next set of numbers, if needed
    for (int j = 0; active_threads; j = (j+1) % THREAD_COUNT) {
        pthread_join(thread_pool[j].id, NULL);
        fwrite(thread_pool[j].buf, thread_pool[j].buflen, 1, stdout);
        if (max - i > NUMS_PER_THREAD) {
            // restart the thread with the full load
            thread_pool[j].start_num = i;
            pthread_create(&thread_pool[j].id, NULL, worker, (void *)&thread_pool[j]);
            i += NUMS_PER_THREAD;
        } else if (max > i) {
            // restart the thread for the remainder
            thread_pool[j].start_num = i;
            thread_pool[j].count = max - i + 1;
            pthread_create(&thread_pool[j].id, NULL, worker, (void *)&thread_pool[j]);
            i += max - i + 1;
        } else {
            // don't restart the thread
            free(thread_pool[j].buf);
            active_threads--;
        }
    } 

    // process tail
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


// worker thread
void *worker(void *arg) {
    int pos[15];
    char wrkbuf[CHUNK_SIZE];
    int wrkbuf_width = 0;   // how many bytes of wrkbuf are actually used
    int digit_num = 0;   // number of digits within the number
    struct thread_data *data = (struct thread_data *)arg;
    int buf_len = 0;

    for (int i = data->start_num; i < data->start_num + data->count; i += 15) {
        wrkbuf_width = process_chunk(i, pos, wrkbuf, wrkbuf_width, &digit_num);
        memcpy(data->buf + buf_len, wrkbuf, wrkbuf_width);
        buf_len += wrkbuf_width;
    }
    data->buflen = buf_len;

    pthread_exit(NULL);
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
        for (int i = pos[I-1] + *digit_num - 1; i >= (int)(pos[I-1] + *digit_num - diff_pos); i--) { \
            wrkbuf[i] = (cur % 10) + '0'; \
            cur /= 10; \
        } \
} while (0)

int process_chunk(int num, int *pos, char *wrkbuf, int wrkbuf_width, int *digit_num) {
    char last_number[NUMBERSIZE] = {0};
    int digit_count = myitoa(num+14, last_number);  // count of decimal digits in the last number in the chunk of 15

    if (*digit_num != digit_count) {
        // there are more digits in the number - fill buffer from the scratch
        char number[NUMBERSIZE+1];  // one extra for newline
        number[NUMBERSIZE] = '\n';
        char *cur = wrkbuf;
        pos[0] = 0;
        NUM;
        if (cur - wrkbuf - 1 == digit_count) {
            *digit_num = digit_count;
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
        return cur - wrkbuf;    // return new used width of work buffer
    }

    // for comparison use first number from current working buffer
    char prev_first_number[NUMBERSIZE] = {0};
    int num_width = pos[1] - 1;
    memcpy(prev_first_number + NUMBERSIZE - num_width, wrkbuf, num_width);

    // find out how many digits actually changed - use SSE2 vector instructions for comparing 16-byte buffers
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

    return wrkbuf_width;     // not changed
}
