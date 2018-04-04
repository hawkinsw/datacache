#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static inline __attribute__((always_inline)) unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("mfence; rdtscp;" : "=a"(lo), "=d"(hi) : : "ecx");
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

static inline __attribute__((always_inline)) void flush(void *) {
	asm("clflush %0" : : "m"(*memory));
}

unsigned long long time_read(unsigned long long *value) {
	register unsigned long long read_value asm ("r12");
	unsigned long long before = 0, after = 0;

	before = rdtsc();
	read_value = value;
	after = rdtsc();

	return after - before;
}

#define CACHE_SIZE 8192
#define ENTRIES    15

int main() {
	uint8_t *memory = NULL;

	posix_memalign((void**)(&memory), CACHE_SIZE, CACHE_SIZE*ENTRIES);

	if (memory == NULL) {
		printf("Failed to allocate memory!\n");
		return 1;
	} else {
		printf("Address: %llx\n", (unsigned long long)memory);
	}

	printf("Access 1: %llu\n", time_read(&global_value));
	printf("Access 2: %llu\n", time_read(&global_value));
	printf("Access 3: %llu\n", time_read(&global_value));
	printf("Access 4: %llu\n", time_read(&global_value));

	return 0;
}
