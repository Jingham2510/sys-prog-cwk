#include "semaphore.h"
#include "stm32f4xx.h"
#include "os.h"
#include <stddef.h>

//Initialies the semaphore structure
void OS_semaphore_init(OS_semaphore_t * const semaphore, const uint32_t start_count){

	semaphore->counter = start_count;	
	
	semaphore->head_waiting_task = NULL;	
	
}


//Attempts to acquire a semaphore token from the semaphore pot
void OS_semaphore_acquire(OS_semaphore_t * const semaphore){

	uint_fast8_t acquired = 0;

	//The task attempts to acquire a token
	while (!acquired){
	
		//Load the current number of tokens in the semaphore and check that it isnt empty
		uint32_t token_count =  __LDREXW(&(semaphore->counter));
		
		//If there are tokens left
		if(token_count > 0){		
		
				//Reduce the token count, and save it 		
				token_count = token_count - 1;
				
				acquired = !(__STREXW(token_count, &(semaphore->counter)));			
								
		}
		//If there are no tokens left
		else{
				//Clear the exclusive access flag - so we can access the semaphore wait list
			__CLREX();
		
			uint_fast8_t stored = 0;
			
			while(!stored){
			
				OS_TCB_t * curr_waiting_task = (OS_TCB_t *) __LDREXW(&(semaphore->head_waiting_task));
				
				//Add the task to the waiting task list
				if(semaphore->head_waiting_task == NULL){			
					stored = !__STREXW( OS_currentTCB(), &(semaphore->head_waiting_task));
				}
				else{
					
					//Go to the last task in the waiting queue
					while(curr_waiting_task->next_task_pointer != NULL){
						curr_waiting_task = curr_waiting_task->next_task_pointer;				
					}
					stored = !__STREXW(OS_currentTCB(), &(curr_waiting_task->next_task_pointer));			
				}
			}		
			//Wait the task
			OS_wait(semaphore, OS_getCheckCode());	
		}	
	}
}


//Adds a token to the semaphore pot
void OS_semaphore_add_token(OS_semaphore_t * const semaphore){

	uint_fast8_t complete = 0;	
	
	//Runs until the token count has been increased
	while(!complete){
	
		uint_fast8_t token_count = __LDREXW(&(semaphore->counter));
		
		token_count = token_count + 1;
		
		complete = !(__STREXW(token_count, &(semaphore->counter)));
		
		if(complete){	

			uint_fast8_t removed = 0;
		
			while(!removed){
		
				OS_TCB_t * task_to_notify = (OS_TCB_t *) __LDREXW(&(semaphore->head_waiting_task));
			
				//Only notify if there are tasks waiting
				if(task_to_notify != NULL){							
					
					//The sempahore then updates the waiting task list (to replace the head)
					if(task_to_notify != NULL){
						removed = !__STREXW(task_to_notify->next_task_pointer, &(semaphore->head_waiting_task));
					}
					else{
						removed = !__STREXW(NULL, &(semaphore->head_waiting_task));
					}
					
					//If the task has been removed from the waiting lis
					if(removed){
						OS_notify(task_to_notify);
					}
				}
				else{
					//No task to notify so exit
					break;
				}
			}
		}	
	}
}
