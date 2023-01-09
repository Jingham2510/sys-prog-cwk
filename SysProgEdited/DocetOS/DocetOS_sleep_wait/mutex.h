#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>

#include "task.h"

//Whats the max amount of waiting tasks?
#define MAX_TASKS

//Definition of a mutex
typedef struct mutex{
	
	OS_TCB_t *TCB_pointer;	
	uint32_t counter;	
	
	//The head of the waiting tasks linked list
	OS_TCB_t * head_waiting_task;
	
	
}OS_mutex_t;


//Creates a mutex
void OS_mutex_init(OS_mutex_t * const mutex);

//Used by the current task to attempt to acquire a given mutex
void OS_mutex_acquire(OS_mutex_t * const mutex);

//Used by the current task to attempt to release a given mutex
void OS_mutex_release(OS_mutex_t * const mutex);


#endif /* MUTEX_H */
