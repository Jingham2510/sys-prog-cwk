#include "circbuffer.h"
#include <string.h>


void OS_circbuffer_init(OS_circbuffer_t * buff){
	
	//Setup the queue to be empty
	for (uint_fast8_t i = 0; i < BUFFSIZE; ++i){
		buff->queue[i] = 0;	
	}	
	
	//Assign the head to point to the head of the buffer
	buff->head = 0;
	
	//Assign the tail to point to the tail of the buffer (which is currently the head)
	buff->tail = 0;	


	//Create the memory pool for the buffer to point to
	mempool_datapacket_t pool_elements[BUFFSIZE];
	OS_pool_init(buff->mempool);
	
	for(uint_fast8_t i =0; i < BUFFSIZE; ++i){
		OS_pool_add(buff->mempool, &pool_elements[i]);
	}	
	
	//Create the mutex and the semaphores
	OS_mutex_init(&buff->mutex);
	
	OS_semaphore_init(&buff->empty_semaphore, BUFFSIZE);
	
	OS_semaphore_init(&buff->full_semaphore, 0);
	
	
	
	

}




void OS_circbuffer_add(OS_circbuffer_t * buff, char data[DATALENGTH]){

	//Acquire a semaphore token to resereve a spot in the queue
	OS_semaphore_acquire(&buff->empty_semaphore);

	//Acquire the mutex so that the task can edit the queue	
	OS_mutex_acquire(&buff->mutex);
		
	//First allocate the data to some space in the memory pool
	mempool_datapacket_t *packet = OS_pool_allocate(buff->mempool);
	//The id is the current task (so the task that is putting the data in the queue)
	packet->id = (uint32_t) OS_currentTCB();
	/* NOTE: CHANGE TO STRNCPY_S ??*/
	strncpy(packet->data, data, DATALENGTH);
	
	
	//Add the pointer to the packet to the queue
	buff->queue[buff->head] = packet;
	
	//Move the head to the next slot
	buff->head = (buff->head + 1) % BUFFSIZE;	
	
	//Release the mutex now that the editing is done
	OS_mutex_release(&buff->mutex);
	
	//Add a token to the full semaphore 
	OS_semaphore_add_token(&buff->full_semaphore);
		
	
	

}


void OS_circbuffer_get(OS_circbuffer_t * buff, mempool_datapacket_t *packet_pointer){

	//Get a token to make sure the buffer isnt empty
	OS_semaphore_acquire(&buff->full_semaphore);
	
	//Acquire the mutex to stop concurrent editing
	OS_mutex_acquire(&buff->mutex);
	
	
	//Dereference the packet and set the packet pointer equal to it
	mempool_datapacket_t packet = *(mempool_datapacket_t *) buff->queue[buff->tail];

	*packet_pointer = packet;
	
	//Free the memory in the memory pool
	OS_pool_deallocate(buff->mempool, buff->queue[buff->tail]);
	
	
	//Move the tail of the buffer
	buff->tail = (buff->tail + 1) % BUFFSIZE;			


	//Release the mutex
	OS_mutex_release(&buff->mutex);
	
	//Add a token to the empty pot
	OS_semaphore_add_token(&buff->full_semaphore);



}













