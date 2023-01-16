#ifndef TASKCOMMSQUEUE_H
#define TASKCOMMSQUEUE_H

#include "circbuffer.h"
#include "mutex.h"
#include "semaphore.h"

//The structure of the intertask communication queue, implemented using a circular buffer 
//Two semaphores and a mutex
typedef struct commsqueue{

	
	//The semaphore which has a number of tokens equal to the number of empty slots in the queue
	OS_semaphore_t empty_semaphore;
	
	OS_semaphore_t full_semaphore;

	OS_mutex_t mutex;
	
	OS_circbuffer_t circbuffer;

}OS_taskcommsqueue_t;





#endif /*TASKCOMMSQUEUE_H*/