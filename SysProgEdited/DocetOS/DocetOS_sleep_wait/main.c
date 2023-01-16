#include "os.h"
#include "sleep.h"
#include <stdio.h>
#include "utils/config.h"
#include "simpleRoundRobin.h"
#include "mutex.h"
#include "semaphore.h"
#include <stm32f4xx.h>
#include "circBuffer.h"
#include "memPool.h"


static OS_mutex_t mutex;

static OS_semaphore_t semaphore;

static OS_circbuffer_t comms_queue;

static OS_mempool_t mempool;

static OS_TCB_t TCB3, TCB4, TCB5; 



void task1(void const * const args) {
	
	
	OS_mutex_acquire(&mutex);
	
	
	OS_circbuffer_init(&comms_queue);	
	
	
	uint32_t test = 10;	

	printf("MEMORY ADDRESS: %p\n", &test);
	printf("VALUE: %d\n", test);
	
	
	uint32_t test2 = 20;
	
	
	OS_circbuffer_add(&comms_queue, &test);
	
	OS_circbuffer_add(&comms_queue, &test2);
	
	OS_mutex_release(&mutex);
	OS_sleep(1000);	

	/*

	OS_mutex_acquire(&mutex);
	
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
	OS_circbuffer_add(&comms_queue, packet);	
	
	OS_mutex_release(&mutex);
	
	*/
	
	OS_mutex_acquire(&mutex);	
		
	for(uint_fast8_t i = 0; i < 25; i++){
		OS_sleep(100);		
		printf("Q1 - Sleep\r\n");				
	}
	
	for(uint_fast8_t i = 0; i < 25; i++){		
		printf("Q1\r\n");				
	}
	
	OS_mutex_release(&mutex);	
}

void task2(void const * const args) {
	
	
	OS_mutex_acquire(&mutex);
		
	uint32_t  * RXTEST = (uint32_t *) OS_circbuffer_get(&comms_queue);
	
	printf("Recieved Pointer: %p\n", RXTEST);
	
	printf("Recieved Pointer Value: %d\n", *RXTEST);
	
	for(uint_fast8_t i = 0; i < *RXTEST; i++){
			printf("%d\n", i);		
	}
	
	OS_addTask(&TCB3);
	OS_addTask(&TCB4);
	OS_addTask(&TCB5);
	
	OS_sleep(10);
	OS_semaphore_add_token(&semaphore);
	
	
	
	OS_mutex_release(&mutex);
	
	
	/*	
	OS_mutex_acquire(&mutex);
	
	mempool_datapacket_t  * packet_pointer = (mempool_datapacket_t *) OS_circbuffer_get(&comms_queue);
		
	printf("Packet pointer: %p\n", packet_pointer);
	
	printf("Packet1 ID: %d \n", packet_pointer->id);
	printf("Packet1 Data: %d \n", packet_pointer->data);
	
	OS_mutex_release(&mutex);
	*/
	
		
}


void task3(void const * const args){

	OS_semaphore_acquire(&semaphore);
	
	for(uint_fast8_t i = 0; i <35; i++){
		OS_sleep(2);
		printf("3");		
	}
	
	OS_semaphore_add_token(&semaphore);
	
}

void task4(void const * const args){
	
	OS_semaphore_acquire(&semaphore);
	
	for(uint_fast8_t i = 0; i <35; i++){
		OS_sleep(2);
		printf("4");		
	}
	
	OS_semaphore_add_token(&semaphore);	

}

void task5(void const * const args){

	OS_semaphore_acquire(&semaphore);
	
	for(uint_fast8_t i = 0; i <10; i++){
		OS_sleep(1);
		printf("5");		
	}
	
	OS_semaphore_add_token(&semaphore);	
}
	



/* MAIN FUNCTION */
int main(void) {
	/* Set up core clock and initialise serial port */
	config_init();

	printf("\r\nDocetOS - Systems Assesment - Y3872776\r\n");

	/* Reserve memory for five stacks and five TCBs.
	   Remember that stacks must be 8-byte aligned. */
	__align(8)
	static uint32_t stack1[128], stack2[128], stack3[128], stack4[128], stack5[128];
	static OS_TCB_t TCB1, TCB2;

	/* Initialise the TCBs using the  functions above */
	OS_initialiseTCB(&TCB1, stack1+64, task1, 0);
	OS_initialiseTCB(&TCB2, stack2+64, task2, 0);
	OS_initialiseTCB(&TCB3, stack3+64, task3, 0);
	OS_initialiseTCB(&TCB4, stack4+64, task4, 0);
	OS_initialiseTCB(&TCB5, stack5+64, task5, 0);


	/* Initialise and start the OS */
	OS_init(&simpleRoundRobinScheduler);
	OS_addTask(&TCB1);
	OS_addTask(&TCB2);	

	OS_mutex_init(&mutex);
	
	OS_semaphore_init(&semaphore, 2);
	
	
	OS_start();
}
