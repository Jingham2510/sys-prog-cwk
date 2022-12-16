#ifndef CIRCBUFFER_H
#define CIRCBUFFER_H

#include "mutex.h"
#include "semaphore.h"
#include <stddef.h>
#include "mempool.h"


/* The buffer size */
#define BUFFSIZE  8


//A circular buffer implemented as a queue of void pointers
//These void pointers point to data packets

/*Structure of the circular buffer*/
typedef struct circbuffer{
	
	//The queue itself
	void * queue[BUFFSIZE];
	
	//The head of the buffer
	uint_fast8_t head;
	
	//The tail of the buffer
	uint_fast8_t tail;
	
	//The memory which the pointers point to 
	OS_mempool_t *mempool;	
	
	//semaphore which has a number of tokens equal to the number of empty slots in the queue
	OS_semaphore_t empty_semaphore;
	
	//Semaphore has a number of tokens equal to the number of full slots in the queue
	OS_semaphore_t full_semaphore;

	//The mutex stops concurrent modification 
	OS_mutex_t mutex;
	
	

}OS_circbuffer_t;


//Initialise the circular buffer
void OS_circbuffer_init(OS_circbuffer_t * buff);


//Add some data to the circular buffer
void OS_circbuffer_add(OS_circbuffer_t * buff, char const * data);


//Remove a pointer from the cicular buffer - and return the data associated with it 
void OS_circbuffer_get(OS_circbuffer_t * buff, mempool_datapacket_t *packet_pointer);




#endif /*CIRCBUFFER_H*/
