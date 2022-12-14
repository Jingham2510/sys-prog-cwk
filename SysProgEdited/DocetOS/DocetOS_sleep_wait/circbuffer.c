#include "circbuffer.h"
#include <string.h>


void circbuffer_init(OS_circbuffer_t * buff){
	
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
	pool_init(&buff->mempool);
	
	for(uint_fast8_t i =0; i < BUFFSIZE; ++i){
		pool_add(&buff->mempool, &pool_elements[i]);
	}	

}




uint_fast8_t circbuffer_add(OS_circbuffer_t * buff, char data[DATALENGTH]){

	//Check to see if the buffer is full
	if( (buff->head % BUFFSIZE) == ((buff->tail & BUFFSIZE) - 1)){
		//Return a failure code
		return 0;
	}
	//If there is space in the buffer
	else{
		
		//First allocate the data to some space in the memory pool
		mempool_datapacket_t *packet = pool_allocate(&buff->mempool);
		//The id is the current task (so the task that is putting the data in the queue)
		packet->id = (uint32_t) OS_currentTCB();
		/* NOTE: CHANGE TO STRNCPY_S ??*/
		strncpy(packet->data, data, DATALENGTH);
		
		
		//Add the pointer to the packet to the queue
		buff->queue[buff->head] = packet;
		
		//Move the head to the next slot
		buff->head = (buff->head + 1) % BUFFSIZE;	
		return 1;
	
	}

}


uint_fast8_t circbuffer_get(OS_circbuffer_t * buff, mempool_datapacket_t *packet_pointer){

	//Check to make sure that the buffer is not empty
	if( buff->head == buff->tail){
		//return a failure code
		return 0;
	}
	else{
		//Dereference the packet and set the packet pointer equal to it
		mempool_datapacket_t packet = *(mempool_datapacket_t *) buff->queue[buff->tail];

		*packet_pointer = packet;
		
		//Move the tail of the buffer
		buff->tail = (buff->tail + 1) % BUFFSIZE;			

		return 1;
	}
}













