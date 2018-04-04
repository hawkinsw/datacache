#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static inline __attribute__((always_inline)) unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtscp;" : "=a"(lo), "=d"(hi) : : "ecx");
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

static inline __attribute__((always_inline)) void flush(void *memory) {
	asm("clflush %0; mfence" : : "m"(*memory));
}

static inline __attribute__((always_inline)) void fence() {
	asm("mfence" : : );
}

unsigned long long time_read(uint8_t *value) {
	register unsigned long long read_value asm ("r12");
	unsigned long long before = 0, after = 0;

	before = rdtsc();
	read_value = *value;
	after = rdtsc();

	return after - before;
}

#define CACHE_SIZE 8192
#define ENTRIES    100
#define WARMUP     100

#define DO_WARMUP(X) \
	do { \
		for (int i = 0; i<WARMUP; i++) { \
			time_read(&(X)); \
		} \
	} while (0);

int main() {
	uint8_t *memory = NULL;

	posix_memalign((void**)(&memory), CACHE_SIZE, CACHE_SIZE*ENTRIES);

	if (memory == NULL) {
		printf("Failed to allocate memory!\n");
		return 1;
	} else {
		printf("Address: %llx\n", (unsigned long long)memory);
	}

	flush(&memory[0]);
	DO_WARMUP(memory[0*CACHE_SIZE]);
	printf("Warm            Access: %llu\n", time_read(&memory[0*CACHE_SIZE]));

	for (int i = 1; i<ENTRIES; i++) {
		memory[i*8192] = 5;
	}

	printf("     Post Write Access: %llu\n", time_read(&memory[0*CACHE_SIZE]));
	DO_WARMUP(memory[0*CACHE_SIZE]);
	printf("Warm Post Write Access: %llu\n", time_read(&memory[0*CACHE_SIZE]));
	return 0;
}
