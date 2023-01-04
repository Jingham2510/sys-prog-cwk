#include "os.h"
#include "sleep.h"
#include <stdio.h>
#include "utils/config.h"
#include "simpleRoundRobin.h"
#include "mutex.h"
#include "semaphore.h"
#include <stm32f4xx.h>
#include "circbuffer.h"
#include "mempool.h"


static OS_mutex_t mutex;

static OS_semaphore_t semaphore;

static OS_circbuffer_t commsqueue; 


static OS_mempool_t mempool;


void task1(void const *const args) {
	OS_mutex_acquire(&mutex);
	
	
	OS_circbuffer_init(&commsqueue);
	/*
	OS_pool_init(&mempool);
	mempool_datapacket_t poolElements[10];
	
	for (int i = 0; i < 10; ++i) {
		OS_pool_add(&mempool, &poolElements[i]);
	}
	mempool_datapacket_t *packet = OS_pool_allocate(&mempool);
	
	
	packet->id = 10;
	packet->data = 123;
	
	printf("POINTER TEST: %p\n",  packet);
	printf("ID TEST %d \n", packet->id);
	printf("DATA TEST %d \n", packet->data);
	OS_circbuffer_add(&commsqueue, packet);	
	*/

	
	
	
	
	uint32_t test = 100;

	printf("%p\n", &test);
	printf("%d\n", test);
	
	OS_circbuffer_add(&commsqueue, &test);
	
	
	
	
	printf("-----Packet added-----\n");
	
	
	
	OS_mutex_release(&mutex);
	
	
	/*
	while(1){
		OS_mutex_acquire(&mutex);
		printf("Q1\r\n");
		OS_mutex_release(&mutex);		
	}
	*/
}

void task2(void const *const args) {
	
	OS_mutex_acquire(&mutex);
	
	uint32_t * test = OS_circbuffer_get(&commsqueue);
	
	printf("TEST: %p\n", test);
	
	printf("TEST: %d\n", *test);
	
	
	/*
	mempool_datapacket_t  * packet_pointer = (mempool_datapacket_t *) OS_circbuffer_get(&commsqueue);
		
	printf("Packet pointer: %p\n", packet_pointer);
	
	printf("Packet1 ID: %d \n", packet_pointer->id);
	printf("Packet1 Data: %d \n", packet_pointer->data);
	*/
	
	
	/*
	for(uint_fast8_t i = 0; i < packet.data; i++){
			printf("%d", i);		
	}
	
	*/
	printf("Packet Read");
	
	OS_mutex_release(&mutex);	
	
	
	/*
	while (1) {		
		OS_mutex_acquire(&mutex);
		printf("Q2\r\n");
		OS_mutex_release(&mutex);		
	}
	*/
	
	
}

/* MAIN FUNCTION */




int main(void) {
	/* Set up core clock and initialise serial port */
	config_init();

	printf("\r\nDocetOS - Systems Assesment - Y3872776\r\n");

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
	
	OS_semaphore_init(&semaphore, 1);
	
	
	
	
	
	OS_start();
}
