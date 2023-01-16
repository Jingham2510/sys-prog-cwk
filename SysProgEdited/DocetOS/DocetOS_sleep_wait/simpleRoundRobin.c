#include "simpleRoundRobin.h"
#include <stm32f4xx.h>
#include "semaphore.h"


/* Prototypes (functions are static, so these aren't in the header file) */
static OS_TCB_t const * simpleRoundRobin_scheduler(void);
static void simpleRoundRobin_addTask(OS_TCB_t * const tcb);
static void simpleRoundRobin_taskExit(OS_TCB_t * const tcb);

static void simpleRoundRobin_wait(void * const reason, uint32_t checkcode);
static void simpleRoundRobin_notify(OS_TCB_t * const task_to_notify);



//The task at the head of the task linked list
static OS_TCB_t * head_active_task;

//Tracks the number of tasks
static uint32_t task_count = 0;
//Tracks of the number of waiting tasks
static uint32_t waiting_task_count = 0;

/* Scheduler block for the simple round-robin */
OS_Scheduler_t const simpleRoundRobinScheduler = {
	.preemptive = 1,
	.scheduler_callback = simpleRoundRobin_scheduler,
	.addtask_callback = simpleRoundRobin_addTask,
	.taskexit_callback = simpleRoundRobin_taskExit,
	.wait_callback = simpleRoundRobin_wait,
	.notify_callback = simpleRoundRobin_notify
	
};


/* Round-robin scheduler callback */
static OS_TCB_t const * simpleRoundRobin_scheduler(void) {
	
	//Get the current TCB
	OS_TCB_t * curr_task = OS_currentTCB();
	
	OS_TCB_t * next_task = curr_task->next_task_pointer;
	
	//Check to see if any task is available to run
	for (uint_fast8_t j = 0; j < (task_count - waiting_task_count); j++){		
		
		//Check to make sure there is a next task
		if(next_task == NULL){
			//If there isnt a next task, loop back to the head task
			next_task = head_active_task;	
		}
		
		//Check to see if the task is sleeping - as we know none of the tasks are waiting
		if(next_task->state & TASK_STATE_SLEEP){	
			
			//If the task is sleeping - check to see if enough time has passed
			if( (int32_t) (OS_elapsedTicks() - next_task->data) > 0){
				//Clear the yield flag and return the task
				next_task->state &= ~TASK_STATE_SLEEP;
				next_task->data = 0;
				return next_task;				
			}	
		}
		else{
			//If the task isn't waiting and isn't sleeping return it			
			return next_task;				
		}			
		next_task = next_task->next_task_pointer;		
	
	}
	// No tasks can be ran in the list, so return the idle task
	return OS_idleTCB_p;
}

/* 'Add task' callback */
static void simpleRoundRobin_addTask(OS_TCB_t * const tcb) {

	if(task_count < SIMPLE_RR_MAX_TASKS){
		
		uint_fast8_t stored = 0;		
		
		while(!stored){

			uint_fast8_t finished = 0;
			//Start the search process at the head task
			OS_TCB_t * curr_task = (OS_TCB_t *) __LDREXW(&head_active_task);
				
			//If there is no head task
			if(head_active_task == NULL){
				stored = !(__STREXW(tcb, &head_active_task));
				task_count = task_count + 1;
				finished = 1; 
			}		
			//Search for the slot at the end of the task list
			while(!finished){			
				if(curr_task->next_task_pointer == NULL){					
					stored = !(__STREXW(tcb, &(curr_task->next_task_pointer)));
					task_count = task_count + 1;
					finished = 1;
				}
				else{					
					curr_task = curr_task->next_task_pointer;
				}
			}
			
		}			
	}		
	// If we get here, there are no free TCB slots, so the task just won't be added
	return;
}

/* 'Task exit' callback */
static void simpleRoundRobin_taskExit(OS_TCB_t * const tcb) {
	
	uint_fast8_t stored = 0;	
	
	while(!stored){
	
		uint_fast8_t exited = 0;
		OS_TCB_t * curr_task = (OS_TCB_t *) __LDREXW(&head_active_task);
		
		//if the task to be exited is the head task
		if(curr_task == head_active_task && curr_task == tcb){					
			stored = !__STREXW(curr_task->next_task_pointer,&head_active_task);
			task_count = task_count - 1;
			exited = 1;
		}		
		
		while(!exited){
			
			//If the next task to be selected is the one to be exited
			if(curr_task->next_task_pointer == tcb){				
				//Change the current tasks next task pointer to the next next task pointer
				stored = !__STREXW((curr_task->next_task_pointer->next_task_pointer), &(curr_task->next_task_pointer));
				task_count = task_count - 1;
				exited = 1;
			}	
			//If the tasks dont match, go to the next task
			else{
				//Make sure the current next task isn't NULL
				if(curr_task->next_task_pointer != NULL){
					curr_task = curr_task->next_task_pointer;
				}
				else{
					//Return as there is no matching task to exit
					return;
				}				
			}	
		}
	}
}



//"wait" task callback
static void simpleRoundRobin_wait(void * const reason, uint32_t checkcode){
	
	if(checkcode == OS_getCheckCode()){
		uint_fast8_t stored = 0;
		
		while(!stored){
			
			OS_TCB_t * curr_task_check = (OS_TCB_t *) __LDREXW(&head_active_task);			
			
			//Remove the current TCB from the ready task list then update its status		
			for(uint_fast8_t i = 0; i <task_count - waiting_task_count; i++){
				//If the task to be waited is the head task
				if(i == 0){
					if(OS_currentTCB() == curr_task_check){								
						stored = !__STREXW(OS_currentTCB()->next_task_pointer,&head_active_task);						
						curr_task_check->next_task_pointer = NULL;						
						waiting_task_count = waiting_task_count + 1;						
					}					
				}
				else{
					//If the next task is the task to be waited - remove it from the list
					if(curr_task_check->next_task_pointer == OS_currentTCB()){
						OS_TCB_t * task_to_remove = curr_task_check->next_task_pointer;
						
						//Set the current tasks next pointer to be the task after the task to be removed
						stored = !__STREXW(task_to_remove->next_task_pointer, &(curr_task_check->next_task_pointer));
						
						//Set the task to be removed pointer to be null
						//Dont need STREXW here because ISRs cannot acquire mutexs/semaphores (and so cannot add anything to waiting lists)
						task_to_remove->next_task_pointer = NULL;
						
						waiting_task_count = waiting_task_count + 1;						
						
					}
					else{
						curr_task_check = curr_task_check->next_task_pointer;
					}
				}		
			}			
		}
		
		OS_currentTCB()->data = (uint32_t) reason;
						
		//Set the waiting flag high
		OS_currentTCB()->state |= TASK_STATE_WAITING;
		
		//Invoke the scheduler to context switch
		SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;		
	}	
}


//"notify" task callback
static void simpleRoundRobin_notify(OS_TCB_t * const task_to_notify){

	//Update the tasks status flags
	task_to_notify->data = 0;
	
	task_to_notify->state &= ~TASK_STATE_WAITING;
	
	uint_fast8_t stored = 0;
	
	while(!stored){
	
		//Put the task back in the active-task linked list
		OS_TCB_t * curr_task = (OS_TCB_t *) __LDREXW(&head_active_task);
		
		uint_fast8_t finished = 0;
		
		//If there are no active tasks
		if (curr_task == NULL){	
			stored = !__STREXW(task_to_notify ,&head_active_task);
			waiting_task_count = waiting_task_count - 1;
			finished = 1;
		}		
		while(!finished){
			//Finds the next slot in the linked list 
			while(curr_task->next_task_pointer != NULL){
				curr_task = curr_task->next_task_pointer;	
			}
			
			stored = !__STREXW(task_to_notify ,&(curr_task->next_task_pointer));
			waiting_task_count = waiting_task_count - 1;
			finished = 1;
		}
	}	
}




