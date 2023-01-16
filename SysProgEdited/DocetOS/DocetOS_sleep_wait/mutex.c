#include "mutex.h"
#include "stm32f4xx.h"
#include "os.h"
#include <stddef.h>


//Initialies a mutex structure
void OS_mutex_init(OS_mutex_t * const mutex){
	
	mutex->counter = 0;
	
  mutex->TCB_pointer = NULL;
	
	mutex->head_waiting_task = NULL;
	
}



//Acquires a mutex
void OS_mutex_acquire(OS_mutex_t * const mutex){
	
	uint_fast8_t acquired = 0;
	
	//A task attempts to acquire the mutex, which it can only do if the mutex is free	
	while (!acquired){	
		
		//Exclusive load of the address of the pointer in the mutex (we dont want to load the data)
		OS_TCB_t * curr_mutex_TCB = (OS_TCB_t *) __LDREXW(&(mutex->TCB_pointer));	
	
		if (curr_mutex_TCB == 0) {			
			acquired = !(__STREXW(OS_currentTCB(), &(mutex->TCB_pointer)));			
			if (acquired == 1){
				mutex->counter += 1;
			}			
		}
		//If the task isn't the owner of the mutex
		else if(curr_mutex_TCB != OS_currentTCB()){		
			//Clear the exclusive access flag - so we can access the mutex wait list
			__CLREX();
			
			uint_fast8_t stored = 0;
			
			while(!stored){
			
				OS_TCB_t * curr_waiting_task = (OS_TCB_t *) __LDREXW(&(mutex->head_waiting_task));
				
				//Add the task to the waiting task list
				if(curr_waiting_task == NULL){			
					stored = !__STREXW(OS_currentTCB() ,&(mutex->head_waiting_task)) ;
				}
				else{					
					//Go to the last task in the waiting queue
					while(curr_waiting_task->next_task_pointer != NULL){
						curr_waiting_task = curr_waiting_task->next_task_pointer;				
					}
					stored = !__STREXW(OS_currentTCB(), &(curr_waiting_task->next_task_pointer));			
				}
			}		
			
			OS_wait(mutex, OS_getCheckCode());				
		}
		else{			
			mutex->counter += 1;
			acquired = 1;
		}
	}	
}



//Releases a mutex 
void OS_mutex_release(OS_mutex_t * const mutex){	

	if(OS_currentTCB() == mutex->TCB_pointer){		
		mutex->counter -= 1;
		
		//If the mutex has been fully freed by its owner
		if (mutex->counter == 0){		
			mutex->TCB_pointer = NULL;		

			uint_fast8_t stored = 0;

			while(!stored){

				OS_TCB_t * task_to_notify = (OS_TCB_t *) __LDREXW(&(mutex->head_waiting_task));

				//Only notify if there are tasks waiting
				if(task_to_notify != NULL){					
								
					//The sempahore then updates the waiting task list (to replace the head)
					if(mutex->head_waiting_task->next_task_pointer != NULL){
						stored = !__STREXW(task_to_notify->next_task_pointer, &(mutex->head_waiting_task));
					}
					else{
						stored = !__STREXW(NULL, &(mutex->head_waiting_task));
					}						
	
					//If the task has been safely removed from the list
					if(stored){
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
