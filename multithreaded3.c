#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

#include "lookup.h"

#define LIMIT 1000000000
#define THREAD_COUNT    4
// must be multiple of 30
#define NUMS_PER_THREAD 90000
#define CHUNK_SIZE 238
#define BUFFER_SIZE (NUMS_PER_THREAD / 30 * CHUNK_SIZE + 1)

struct thread_data;	// forward struct declaration
typedef void (*worker_func)(struct thread_data *);

struct thread_data {
    pthread_t   id;
    int         first;
    int         last;
    char        *buf;
    int         buflen;
	worker_func	worker;
	sem_t 		new_task;
	sem_t 		idle;
};
struct thread_data thread_pool[THREAD_COUNT];

static void *thread_func(void *arg);
static void tail(struct thread_data *data);      // slow worker for processing tail
static void medium(struct thread_data *data);    // average-speed worker for chunks of 15
static void fast8(struct thread_data *data);     // fast worker 8-digit numbers
static void fast9(struct thread_data *data);     // fast worker 8-digit numbers

struct job {
	int     	first;
	int     	last;
	worker_func	proc;
} jobs[] = {
	{1,         10000020,   medium},
	{10000021,  99999990,   fast8},    // 90M (9%)
	{99999991,  100000020,  medium},
	{100000021, 999999990,  fast9},    // 900M (90%)
	{999999991, 1000000000, tail}
};

int main(void) {
    int next = 1, job = 0;
    // start all workers
    for (int j = 0; j < THREAD_COUNT; j++) {
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
		thread_pool[j].buflen = 0;
		thread_pool[j].worker = jobs[job].proc;
		sem_init(&thread_pool[j].new_task, 0, 1);	// there is already a task to process
		sem_init(&thread_pool[j].idle, 0, 0);
        pthread_create(&thread_pool[j].id, NULL, thread_func, (void *)&thread_pool[j]);
    }

    int active_threads = THREAD_COUNT;
    // wait for worker result, print result and give a new chunk to the worker
    for (int j = 0; active_threads; j = (j+1) % THREAD_COUNT) {
		sem_wait(&thread_pool[j].idle);
		write(1, thread_pool[j].buf, thread_pool[j].buflen);
		if (job < sizeof(jobs)/sizeof(jobs[0])) {
			thread_pool[j].first = next;
			thread_pool[j].worker = jobs[job].proc;
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
			sem_post(&thread_pool[j].new_task);
		} else {
			// don't resume the thread
            active_threads--;
		}
	}

	return 0;
}

void *thread_func(void *arg) {
	struct thread_data *data = (struct thread_data *)arg;
	for (;;) {
		sem_wait(&data->new_task);
		data->worker(data);
		sem_post(&data->idle);
	}
	return NULL;
}

// ----- slow - tail only -----
void tail(struct thread_data *data) {
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
}

// ----- medium-speed - good for processing chunks with variable number size -----
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

void medium(struct thread_data *data) {
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
}

#undef NUM
#undef FIZZ
#undef BUZZ
#undef FIZZBUZZ
#undef FIZZ_BUZZ
#undef BUZZ_FIZZ

// ----- fastest worker for 9-digit numbers -----
#define NUMPOS(A) *((uint64_t *)(cur+A)) = number.whole;
#define NORM do { l++; if (__builtin_expect(l == 10000, 0)) { l = 0; h += 1; number.high = table10K[h]; } number.low = table10K[l]; } while(0)

char pattern[CHUNK_SIZE] = {	// offset
	"000000001\n"				// 0
	"000000002\n"				// 10
	"Fizz\n"					// 20
	"000000004\n"				// 25
	"Buzz\n"					// 35
	"Fizz\n"					// 40
	"000000007\n"				// 45
	"000000008\n"				// 55
	"Fizz\n"					// 65
	"Buzz\n"					// 70
	"000000001\n"				// 75
	"Fizz\n"					// 85
	"000000003\n"				// 90
	"000000004\n"				// 100
	"FizzBuzz\n"				// 110
	"000000006\n"				// 119
	"000000007\n"				// 129
	"Fizz\n"					// 139
	"000000009\n"				// 144
	"Buzz\n"					// 154
	"Fizz\n"					// 159
	"000000002\n"				// 164
	"000000003\n"				// 174
	"Fizz\n"					// 184
	"Buzz\n"					// 189
	"000000006\n"				// 194
	"Fizz\n"					// 204
	"000000008\n"				// 209
	"000000009\n"				// 219
	"FizzBuzz\n"				// 229
};

union numbuf {
	uint64_t whole;
	struct {
		uint32_t high;
		uint32_t low;
	};
};

void fast9(struct thread_data *data) {
	char *cur = data->buf;

	if (data->buflen < (data->last - data->first + 1) / 30 * sizeof(pattern)) {
		for (int number = data->first; number < data->last; number += 30) {
			memcpy(cur, pattern, sizeof(pattern));
			cur += sizeof(pattern);
		}
		cur = data->buf;
	}

	int first_digits = data->first / 10;
	int h = first_digits / 10000;    // decimal digits 1-4
	int l = first_digits % 10000;    // decimal digits 5-8
	union numbuf number;
	number.high = table10K[h];
	number.low = table10K[l];

	for (int i = data->first; i <= data->last; i += 30) {
		NUMPOS(0);		// 1
		NUMPOS(10);		// 2
		NUMPOS(25);		// 4
		NUMPOS(45);		// 7
		NUMPOS(55);		// 8
		NORM;

		NUMPOS(75);		// 11
		NUMPOS(90);		// 13
		NUMPOS(100);	// 14
		NUMPOS(119);	// 16
		NUMPOS(129);	// 17
		NUMPOS(144);	// 19
		NORM;

		NUMPOS(164);	// 22
		NUMPOS(174);	// 23
		NUMPOS(194);	// 26
		NUMPOS(209);	// 28
		NUMPOS(219);	// 29
		NORM;

		cur += sizeof(pattern);
	}

	data->buflen = cur - data->buf;
}

#undef NORM

// ----- fast worker for 8-digit numbers -----
#define FIZZ do { *((uint64_t *)cur) = 0x0a7a7a6946; cur += 5; } while (0)
#define BUZZ do { *((uint64_t *)cur) = 0x0a7a7a7542; cur += 5; } while (0)
#define FIZZBUZZ do { *((uint64_t *)cur) = 0x7a7a75427a7a6946; cur += 8; *((uint8_t *)cur) = 0x0a; cur++; } while (0)
#define FIZZ_BUZZ do { *((uint64_t *)cur) = 0x7a75420a7a7a6946; cur += 8; *((uint16_t *)cur) = 0x0a7a; cur += 2; } while (0)
#define BUZZ_FIZZ do { *((uint64_t *)cur) = 0x7a69460a7a7a7542; cur += 8; *((uint16_t *)cur) = 0x0a7a; cur += 2; } while (0)
#define NUM do {*((uint32_t *)cur) = high; cur += 3; *((uint32_t *)cur) = low; cur += 4; } while (0)
#define NORM do { l++; if (__builtin_expect(l == 10000, 0)) { l -= 10000; h += 1; high = table1K[h]; } low = table10K[l]; } while(0)
#define DIGIT(A) do {*cur = A; cur++; *cur = '\n'; cur ++; } while (0)

void fast8(struct thread_data *data) {
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
}
