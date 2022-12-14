#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include "os.h"


//What is the max amount of waiting tasks?
#define SEM_MAX_TASKS 8 


//Definition of a sempahore
typedef struct semaphore{

	//A counter of the number of tokens
	uint32_t counter;
	
	//Keeps a list of the current waiting tasks
	OS_TCB_t * waiting_tasks[SEM_MAX_TASKS];

	//Keeps count of how many tasks are waiting
	uint_fast8_t waiting_count;

}OS_semaphore_t;

//Creates a semaphore with the given token counter of "start_count"
void OS_semaphore_init(OS_semaphore_t * semaphore, uint32_t start_count);

//Used by the current task to attempt to acquire a semaphore token
void OS_semaphore_acquire(OS_semaphore_t * semaphore);

//Used by the current task to add a token to the semaphore counter
void OS_semaphore_add_token(OS_semaphore_t * semaphore); 


#endif /* SEMAPHORE_H */
