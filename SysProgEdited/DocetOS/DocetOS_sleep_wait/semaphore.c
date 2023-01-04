#include "semaphore.h"
#include "stm32f4xx.h"
#include "os.h"
#include <stddef.h>

//Initialies the semaphore structure
void OS_semaphore_init(OS_semaphore_t * semaphore, uint32_t start_count){

	semaphore->counter = start_count;
	
	
	semaphore->head_waiting_task = NULL;
	
	
}


//Attempts to acquire a semaphore token from the semaphore pot
void OS_semaphore_acquire(OS_semaphore_t * semaphore){

	uint_fast8_t complete = 0;

	//The task attempts to acquire a token
	while (!complete){
	
		//Load the current number of tokens in the semaphore and check that it isnt empty
		uint32_t token_count =  __LDREXW(&(semaphore->counter));
		
		//printf("%d", token_count);
		
		//If there are tokens left
		if(token_count > 0){
		
				
		
				//Reduce the token count, and save it 		
				token_count = token_count - 1;
				
				complete = !(__STREXW(token_count, &(semaphore->counter)));			
								
		}
		//If there are no tokens left
		else{
			//Clear the exclusive access flag - we dont edit the token count
			__CLREX();	
			
			
			
			//Add the task to the waiting task list
			if(semaphore->head_waiting_task == NULL){			
				semaphore->head_waiting_task = OS_currentTCB();			
			}
			else{
				OS_TCB_t * curr_task = semaphore->head_waiting_task;
				//Go to the last task in the waiting queue
				while(curr_task->next_task_pointer != NULL){
					curr_task = curr_task->next_task_pointer;				
				}
				curr_task->next_task_pointer = OS_currentTCB();			
			}			
			
			//Wait the task
			OS_wait(semaphore, OS_getCheckCode());	
				
		}	
	}
}


//Adds a token to the semaphore pot
void OS_semaphore_add_token(OS_semaphore_t * semaphore){

	uint_fast8_t complete = 0;	
	
	//Runs until the token count has been increased
	while(!complete){
	
		uint_fast8_t token_count = __LDREXW(&(semaphore->counter));
		
		token_count = token_count + 1;
		
		complete = !(__STREXW(token_count, &(semaphore->counter)));
		
		if(complete){		
			
				//Only notify if there are tasks waiting
				if(semaphore->head_waiting_task != NULL){
					
					
					//printf("NOTIFYING");
					
					OS_TCB_t * task_to_notify = semaphore->head_waiting_task;						
					
					//The sempahore then updates the waiting task list (to replace the head)
					if(semaphore->head_waiting_task->next_task_pointer != NULL){
						semaphore->head_waiting_task = semaphore->head_waiting_task->next_task_pointer;
					}
					else{
						semaphore->head_waiting_task = NULL;
					}
					
					//The OS removes the waiting flag from the task
					OS_notify(task_to_notify);
				
			}
		}
	
	}
	
	





}
