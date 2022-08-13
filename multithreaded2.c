#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#include "lookup.h"

#define LIMIT 1000000000
// 4 worker threads with load of 3M numbers per thread give best result on Core i7-7600U. YMMV
#define THREAD_COUNT    4
// must be multiple of 30
#define NUMS_PER_THREAD 3000000
#define CHUNK_SIZE 238
// per-thread buffer size ~ 22.7 MB (4 threads, 3M numbers load)
#define BUFFER_SIZE (NUMS_PER_THREAD / 30 * CHUNK_SIZE + 1)

typedef void *(*worker)(void *);

struct thread_data {
    pthread_t   id;
    int         first;
    int         last;
    char        *buf;
    int         buflen;
};
struct thread_data thread_pool[THREAD_COUNT];

static void *tail (void *arg);      // slow worker for processing tail
static void *normal (void *arg);    // average-speed worker for chunks of 15
static void *fast8 (void *arg);     // fast worker 8-digit numbers
static void *fast9 (void *arg);     // fast worker 8-digit numbers

struct job {
	int    first;
	int    last;
	worker proc;
} jobs[] = {
	{1,         10000020,   normal},
	{10000021,  99999990,   fast8},    // 9%
	{99999991,  100000020,  normal},
	{100000021, 999999990,  fast9},    // 90%
	{999999991, 1000000000, tail},
};

int main(void) {
    int next = 1, job = 0;
    // start all workers
    for (int j = 0; j < THREAD_COUNT; j++) {
		worker proc = jobs[job].proc;
        thread_pool[j].first = next;
		int end = next + NUMS_PER_THREAD - 1;
		if (end > jobs[job].last) {
			end = jobs[job].last;
			job++;
			next = jobs[job].first;
		} else {
			next = end + 1;
		}
        thread_pool[j].last = end;
        thread_pool[j].buf = malloc(BUFFER_SIZE);
        pthread_create(&thread_pool[j].id, NULL, proc, (void *)&thread_pool[j]);
    }

    int active_threads = THREAD_COUNT;
    // wait for workers, print result and start worker again for next set of numbers, if needed
    for (int j = 0; active_threads; j = (j+1) % THREAD_COUNT) {
        pthread_join(thread_pool[j].id, NULL);
        fwrite(thread_pool[j].buf, thread_pool[j].buflen, 1, stdout);
		if (job < sizeof(jobs)/sizeof(jobs[0])) {
			// restart the thread with a new piece of work
			worker proc = jobs[job].proc;
			thread_pool[j].first = next;
			int end = next + NUMS_PER_THREAD - 1;
			if (end >= jobs[job].last) {
				end = jobs[job].last;
				job++;
				if (job < sizeof(jobs)/sizeof(jobs[0])) {
					next = jobs[job].first;
				}
			} else {
				next = end + 1;
			}
			thread_pool[j].last = end;
			pthread_create(&thread_pool[j].id, NULL, proc, (void *)&thread_pool[j]);
		} else {
			// don't restart the thread
            free(thread_pool[j].buf);
            active_threads--;
		}
	}

	return 0;
}

// ===== slow workers =====

// ----- slowest - tail only -----
void *tail(void *arg) {
	struct thread_data *data = (struct thread_data *)arg;
	char *cur = data->buf;

	for (int i = data->first; i <= data->last; i++) {
        if (i % 3 == 0) {
            memcpy(cur, "Fizz\n", 5);
			cur += 5;
        } else if (i % 5 == 0) {
            memcpy(cur, "Buzz\n", 5);
			cur += 5;
        } else {
            cur += sprintf(cur, "%d\n", i);
        }
	}

	data->buflen = cur - data->buf;
	pthread_exit(NULL);
}

// ----- normal -----

static int myitoa(int number, char *buf) {
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

#define NUM cur += myitoa(i++, cur)
#define FIZZ do { memcpy(cur, "Fizz\n", 5); cur += 5; i++; } while (0)
#define BUZZ do { memcpy(cur, "Buzz\n", 5); cur += 5; i++; } while (0)
#define FIZZBUZZ do { memcpy(cur, "FizzBuzz\n", 9); cur += 9; i++; } while (0)
#define FIZZ_BUZZ do { memcpy(cur, "Fizz\nBuzz\n", 10); cur += 10; i += 2; } while (0)
#define BUZZ_FIZZ do { memcpy(cur, "Buzz\nFizz\n", 10); cur += 10; i += 2; } while (0)

void *normal(void *arg) {
	struct thread_data *data = (struct thread_data *)arg;
	char *cur = data->buf;

	for (int i = data->first; i <= data->last;) {
		NUM;		// 1
		NUM;		// 2
		FIZZ;		// 3
		NUM;		// 4
		BUZZ_FIZZ;	// 5, 6
		NUM;		// 7
		NUM;		// 8
		FIZZ_BUZZ;	// 9, 10
		NUM;		// 11
		FIZZ;		// 12
		NUM;		// 13
		NUM;		// 14
		FIZZBUZZ;	// 15
	}

	data->buflen = cur - data->buf;
	pthread_exit(NULL);
}
#undef NUM
#undef FIZZ
#undef BUZZ
#undef FIZZBUZZ
#undef FIZZ_BUZZ
#undef BUZZ_FIZZ

// ===== fast workers =====
#define FIZZ do { *((uint64_t *)cur) = 0x0a7a7a6946; cur += 5; } while (0)
#define BUZZ do { *((uint64_t *)cur) = 0x0a7a7a7542; cur += 5; } while (0)
#define FIZZBUZZ do { *((uint64_t *)cur) = 0x7a7a75427a7a6946; cur += 8; *((uint8_t *)cur) = 0x0a; cur++; } while (0)
#define FIZZ_BUZZ do { *((uint64_t *)cur) = 0x7a75420a7a7a6946; cur += 8; *((uint16_t *)cur) = 0x0a7a; cur += 2; } while (0)
#define BUZZ_FIZZ do { *((uint64_t *)cur) = 0x7a69460a7a7a7542; cur += 8; *((uint16_t *)cur) = 0x0a7a; cur += 2; } while (0)
#define DIGIT(A) do {*cur = A; cur++; *cur = '\n'; cur ++; } while (0)

// ----- fast9 -----
#define NUM do {*((uint32_t *)cur) = high; cur += 4; *((uint32_t *)cur) = low; cur += 4; } while (0)
#define NORM do { l++; if (l >= 10000) { l -= 10000; h += 1; high = table10K[h]; } low = table10K[l]; } while(0)

void *fast9(void *arg) {
	struct thread_data *data = (struct thread_data *)arg;
	char *cur = data->buf;

	int first_digits = data->first / 10;
	int h = first_digits / 10000;    // decimal digits 1-4
	int l = first_digits % 10000;    // decimal digits 5-8
	uint32_t high = table10K[h];
	uint32_t low = table10K[l];
	for (int i = data->first; i <= data->last; i += 30) {
		NUM; DIGIT('1');	// 1
		NUM; DIGIT('2');	// 2
		FIZZ;				// 3
		NUM; DIGIT('4');	// 4
		BUZZ_FIZZ;			// 5, 6
		NUM; DIGIT('7');	// 7
		NUM; DIGIT('8');	// 8
		FIZZ_BUZZ;			// 9, 10
		NORM;
		NUM; DIGIT('1');	// 11
		FIZZ;				// 12
		NUM; DIGIT('3');	// 13
		NUM; DIGIT('4');	// 14
		FIZZBUZZ;			// 15
		NUM; DIGIT('6');	// 16
		NUM; DIGIT('7');	// 17
		FIZZ;				// 18
		NUM; DIGIT('9');	// 19
		BUZZ_FIZZ;			// 20, 21
		NORM;
		NUM; DIGIT('2');	// 22
		NUM; DIGIT('3');	// 23
		FIZZ_BUZZ;			// 24, 25
		NUM; DIGIT('6');	// 26
		FIZZ;				// 27
		NUM; DIGIT('8');	// 28
		NUM; DIGIT('9');	// 29
		FIZZBUZZ;			// 30
		NORM;
	}

	data->buflen = cur - data->buf;
	pthread_exit(NULL);
}

#undef NUM
#undef NORM

// ----- fast8 -----
#define NUM do {*((uint32_t *)cur) = high; cur += 3; *((uint32_t *)cur) = low; cur += 4; } while (0)
#define NORM do { l++; if (l >= 10000) { l -= 10000; h += 1; high = table1K[h]; } low = table10K[l]; } while(0)

void *fast8(void *arg) {
	struct thread_data *data = (struct thread_data *)arg;
	char *cur = data->buf;

	int first_digits = data->first / 10;
	int h = first_digits / 10000;    // decimal digits 1-3
	int l = first_digits % 10000;    // decimal digits 4-7
	uint32_t high = table1K[h];
	uint32_t low = table10K[l];
	for (int i = data->first; i <= data->last; i += 30) {
		NUM; DIGIT('1');	// 1
		NUM; DIGIT('2');	// 2
		FIZZ;				// 3
		NUM; DIGIT('4');	// 4
		BUZZ_FIZZ;			// 5, 6
		NUM; DIGIT('7');	// 7
		NUM; DIGIT('8');	// 8
		FIZZ_BUZZ;			// 9, 10
		NORM;
		NUM; DIGIT('1');	// 11
		FIZZ;				// 12
		NUM; DIGIT('3');	// 13
		NUM; DIGIT('4');	// 14
		FIZZBUZZ;			// 15
		NUM; DIGIT('6');	// 16
		NUM; DIGIT('7');	// 17
		FIZZ;				// 18
		NUM; DIGIT('9');	// 19
		BUZZ_FIZZ;			// 20, 21
		NORM;
		NUM; DIGIT('2');	// 22
		NUM; DIGIT('3');	// 23
		FIZZ_BUZZ;			// 24, 25
		NUM; DIGIT('6');	// 26
		FIZZ;				// 27
		NUM; DIGIT('8');	// 28
		NUM; DIGIT('9');	// 29
		FIZZBUZZ;			// 30
		NORM;
	}

	data->buflen = cur - data->buf;
	pthread_exit(NULL);
}
