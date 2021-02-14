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
int process_chunk(int num, char *pos[8], char *wrkbuf, int wrkbuf_width, int *digit_num);

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
    char wrkbuf[CHUNK_SIZE] = {
        [CHUNK_SIZE - 10] = '\n', 'F', 'i', 'z', 'z', 'B', 'u', 'z', 'z', '\n'
    };
    char *pos[8];           // past the end positions of numbers within the buffer
    int buf_len = 0;        // output length
    int digit_num = 0;      // number of digits within the number
    int wrkbuf_width = 0;   // how many bytes of wrkbuf are actually used

    struct thread_data *data = (struct thread_data *)arg;

    for (int i = data->start_num; i < data->start_num + data->count; i += 15) {
        wrkbuf_width = process_chunk(i, pos, wrkbuf, wrkbuf_width, &digit_num);
        memcpy(data->buf + buf_len, wrkbuf + CHUNK_SIZE - wrkbuf_width, wrkbuf_width);
        buf_len += wrkbuf_width;
    }
    data->buflen = buf_len;

    pthread_exit(NULL);
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

int process_chunk(int num, char *pos[8], char *wrkbuf, int wrkbuf_width, int *digit_num) {
    // always output num + 13 to check number of digits
    char *cur = myitoa14(num + 13, wrkbuf + CHUNK_SIZE - 10, pos[7]);    // 14

    if (*digit_num != wrkbuf + CHUNK_SIZE - 10 - cur) {
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
        *digit_num = pos[1] - cur;
        return wrkbuf + CHUNK_SIZE - cur;
    }

    // find out how many digits actually changed - use SSE2 instruction for comparing 8-byte buffers
    unsigned int diff = ~_mm_movemask_epi8(_mm_cmpeq_epi8(
                                            _mm_loadl_epi64((__m128i const *)pos[0]),
                                            _mm_loadl_epi64((__m128i const *)cur)));
    // lower zero bits indicate unchanged bytes
    unsigned int diff_len = *digit_num - _tzcnt_u32(diff);   // number of changed digits

    myitoa_diff(num + 12, cur - 1, diff_len);   // 13
    myitoa_diff(num + 10, pos[6] , diff_len);   // 11
    myitoa_diff(num +  7, pos[5] , diff_len);   // 8
    myitoa_diff(num +  6, pos[4] , diff_len);   // 7
    myitoa_diff(num +  3, pos[3] , diff_len);   // 4
    myitoa_diff(num +  1, pos[2] , diff_len);   // 2
    myitoa_diff(num +  0, pos[1] , diff_len);   // 1

    return wrkbuf_width;     // not changed
}
