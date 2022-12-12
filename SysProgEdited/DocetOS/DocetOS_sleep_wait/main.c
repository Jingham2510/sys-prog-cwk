#include "os.h"
#include "sleep.h"
#include <stdio.h>
#include "utils/config.h"
#include "simpleRoundRobin.h"
#include "mutex.h"


static OS_mutex_t mutex;


void task1(void const *const args) {
	while (1) {
		OS_mutex_acquire(&mutex);
		printf("Message from Task 1\r\n");
		OS_mutex_release(&mutex);		
	}
}

void task2(void const *const args) {
	while (1) {		
		OS_mutex_acquire(&mutex);
		printf("Message from Task 2\r\n");		
		OS_mutex_release(&mutex);
	}
}

/* MAIN FUNCTION */




int main(void) {
	/* Set up core clock and initialise serial port */
	config_init();

	printf("\r\nDocetOS Sleep and Mutex\r\n");

	/* Reserve memory for two stacks and two TCBs.
	   Remember that stacks must be 8-byte aligned. */
	__align(8)
	static uint32_t stack1[128], stack2[128];
	static OS_TCB_t TCB1, TCB2;

	/* Initialise the TCBs using the two functions above */
	OS_initialiseTCB(&TCB1, stack1+64, task1, 0);
	OS_initialiseTCB(&TCB2, stack2+64, task2, 0);

	/* Initialise and start the OS */
	OS_init(&simpleRoundRobinScheduler);
	OS_addTask(&TCB1);
	OS_addTask(&TCB2);
	
	OS_mutex_init(&mutex);
	
	
	OS_start();
}
