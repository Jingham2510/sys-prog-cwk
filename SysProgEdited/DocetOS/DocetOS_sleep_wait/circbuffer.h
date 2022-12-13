#ifndef CIRCBUFFER_H
#define CIRCBUFFER_H

#include "mutex.h"
#include "semaphore.h"
#include <stddef.h>


//A circular buffer implemented as a queue of void pointers

/*Structure of the circular buffer*/
typedef struct circbuffer{
	//The head of the buffer
	void *head;
	
	//The tail of the buffer
	void *tail;

	//The size of the circular buffer
	uint_fast8_t buffsize;	

}OS_circbuffer_t;


//Initialise the circular buffer
void circbuffer_init(OS_circbuffer_t * buff);


//Add a pointer to the circular buffer
void circbuffer_add(OS_circbuffer_t * buff, void *item);


//Remove a pointer from the cicular buffer
void *circbuffer_get(OS_circbuffer_t * buff);




#endif /*CIRCBUFFER_H*/