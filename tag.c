#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#ifdef USEMSR
#include <assert.h>
#include <iostream>
#include "msr.hpp"
#endif

#define CPU_NO 1
#define ASSOC 0
extern int errno;

/*
 * __i and __j are required to keep the global_accessed_value
 * from being loaded in when the program starts. Not sure why
 * they are prefetched, but padding the front and back get
 * rid of the problem. The other option is to clflush before 
 * any real work is done, but I like this approach better.
 */
unsigned long long __i[1024] = {5,};
/*
 * Globals that will be used for purposes of the test.
 */
unsigned long long global_accessed_value = 5;
unsigned long long __j[1024] = {5,};

/*
 * This is a function that makes it more convenient to 
 * invoke the rdstscp instruction.
 */
static inline __attribute__((always_inline)) unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("mfence; rdtscp;" : "=a"(lo), "=d"(hi) : : "ecx");
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

void writers(void) {
	unsigned long long before = 0, after = 0;
	unsigned long long register read_value asm ("r12") = 0;
#if ASSOC>0
	int writer_pids[ASSOC] = {0,};
 	for (int wp = 0; wp<ASSOC; wp++)
	{
		int writer_pid = 0;
		if (!(writer_pid= fork())) {
			before = rdtsc();	
			global_accessed_value = before;
			after = rdtsc();	

			//asm("clflush %0" : : "m"(global_accessed_value));

			exit(0);
		}
		writer_pids[wp] = writer_pid;
	}
	for (int wp = 0; wp<ASSOC; wp++) {
		waitpid(writer_pids[wp], NULL, 0);
	}
#endif
}

void reader(void) {
	unsigned long long before = 0, after = 0;
	unsigned long long register read_value asm ("r12") = 0;
	before = rdtsc();	
	read_value = global_accessed_value;
	after = rdtsc();	

	printf("reader time: %llu\n", (after - before));
}

int main() {
	unsigned long long register read_value asm ("r12") = 0;
	unsigned long long before = 0, after = 0;
	cpu_set_t cpu_mask;

	CPU_ZERO(&cpu_mask);
	CPU_SET(CPU_NO, &cpu_mask);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);

	/*
	 * warm up the cache and then tell me how long it takes to read the value.
	 */
	for (int warmup = 0; warmup<5; warmup++) {
		before = rdtsc();
		read_value = global_accessed_value;
		after = rdtsc();
		printf("Warmup read takes: %llu\n", (after - before));
	}

	before = rdtsc();
	read_value = global_accessed_value;
	after = rdtsc();

	writers();
	reader();

	return 0;
}
