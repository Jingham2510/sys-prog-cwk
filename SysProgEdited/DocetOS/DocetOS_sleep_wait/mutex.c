#include "mutex.h"
#include "stm32f4xx.h"
#include "os.h"
#include <stddef.h>


//Initialies a mutex structure
void OS_mutex_init(OS_mutex_t * mutex){
	
	mutex->counter = 0;
	
  mutex->TCB_pointer = NULL;
	
	mutex->head_waiting_task = NULL;
	
}



//Acquires a mutex
void OS_mutex_acquire(OS_mutex_t * mutex){
	
	uint_fast8_t complete = 0;
	
	//A task attempts to acquire the mutex, which it can only do if the mutex is free	
	while (!complete){	
		
		//Exclusive load of the address of the pointer in the mutex (we dont want to load the data)
		OS_TCB_t *curr_mutex_TCB = (OS_TCB_t *) __LDREXW(&(mutex->TCB_pointer));	
	
		if (curr_mutex_TCB == 0) {			
			complete = !(__STREXW(OS_currentTCB(), &(mutex->TCB_pointer)));			
			if (complete == 1){
					mutex->counter += 1;
			}			
		}else if(curr_mutex_TCB != OS_currentTCB()){		
			
			
			//Add the task to the waiting task list
			if(mutex->head_waiting_task == NULL){			
				mutex->head_waiting_task = OS_currentTCB();
				}
			else{
				OS_TCB_t * curr_task = mutex->head_waiting_task;
				//Go to the last task in the waiting queue
				while(curr_task->next_task_pointer != NULL){
					curr_task = curr_task->next_task_pointer;				
				}
				curr_task->next_task_pointer = OS_currentTCB();			
			}			
			
			
			OS_wait(mutex, OS_getCheckCode());				
		}else{			
			__CLREX();			
			mutex->counter += 1;
			complete = 1;
			
		}
	}
	
}



//Releases a mutex
void OS_mutex_release(OS_mutex_t * mutex){	
	
	if(OS_currentTCB() == mutex->TCB_pointer){		
		mutex->counter -= 1;
		if (mutex->counter == 0){		
			mutex->TCB_pointer = NULL;		
			
				//Only notify if there are tasks waiting
				if(mutex->head_waiting_task != NULL){
					
					
					//printf("NOTIFYING");
					
					OS_TCB_t * task_to_notify = mutex->head_waiting_task;						
					
					//The sempahore then updates the waiting task list (to replace the head)
					if(mutex->head_waiting_task->next_task_pointer != NULL){
						mutex->head_waiting_task = mutex->head_waiting_task->next_task_pointer;
					}
					else{
						mutex->head_waiting_task = NULL;
					}
					
			
					OS_notify(task_to_notify);		
			}					
		}
	}	
}
