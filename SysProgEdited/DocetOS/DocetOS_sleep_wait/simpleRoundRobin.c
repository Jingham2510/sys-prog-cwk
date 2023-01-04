#include "simpleRoundRobin.h"
#include <stm32f4xx.h>
#include "semaphore.h"


/* Prototypes (functions are static, so these aren't in the header file) */
static OS_TCB_t const * simpleRoundRobin_scheduler(void);
static void simpleRoundRobin_addTask(OS_TCB_t * const tcb);
static void simpleRoundRobin_taskExit(OS_TCB_t * const tcb);

static void simpleRoundRobin_wait(void * const reason, uint32_t checkcode);
static void simpleRoundRobin_notify(OS_TCB_t * const task_to_notify);



//The task at the head of the task linked list - if none, set to idle
static OS_TCB_t *head_task;

//A count to keep track of the number of tasks
static uint32_t task_count = 0;
//A count to keep track of the number of waiting tasks
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
	
	OS_TCB_t * next_task;
	
	//Check to see if any task is available to run
	for (uint_fast8_t j = 1; j <= (task_count - waiting_task_count); j++){
	
		//Check to make sure there is a next task
		if(curr_task->next_task_pointer == NULL){
			//If there isnt a next task, loop back to the head task
			next_task = head_task;	
		}
		else{
			next_task = curr_task->next_task_pointer;
		}		
		
		//Check to see if the task is sleeping/yielding - as we know none of the tasks are waiting
		if(next_task->state & TASK_STATE_YIELD){
			//If the task is sleeping - check to see if enough time has passed
			if( (int32_t) (OS_elapsedTicks() - next_task->data) > 0){
				//Clear the yeild flag and return the task
				next_task->state &= ~TASK_STATE_YIELD;
				return next_task;				
			}			
		}
		else{
			//If the task isn't waiting and isn't sleeping return it
			return next_task;				
		}	
	
	}
	// No tasks can be ran in the list, so return the idle task
	return OS_idleTCB_p;
}

/* 'Add task' callback */
static void simpleRoundRobin_addTask(OS_TCB_t * const tcb) {
	if(task_count == 0){
		head_task = tcb;
		task_count = task_count + 1;
	}
	else if(task_count < SIMPLE_RR_MAX_TASKS){
		//A boolean to keep track of whether the task has been linked
		uint_fast8_t linked = 0;
		
		//Start the search process at the head task
		OS_TCB_t * curr_task = head_task;
		
		while(!linked){		
				
				if(curr_task->next_task_pointer == NULL){
					curr_task->next_task_pointer = tcb;
					task_count = task_count + 1;
					linked = 1;
				}
				else{
					curr_task = curr_task->next_task_pointer;
				}
			
			}			
		}
		
	// If we get here, there are no free TCB slots, so the task just won't be added
	return;
}

/* 'Task exit' callback */
static void simpleRoundRobin_taskExit(OS_TCB_t * const tcb) {
	// Remove the given TCB from the list of tasks so it won't be run again
	//Boolean to keep track of when the task has been removed from the link list
	uint_fast8_t exited = 0;
	
	OS_TCB_t * curr_task = head_task;
	
	while(!exited){			
				
			//if the task to be exited is the head task
			if(curr_task == head_task && curr_task == tcb){					
				head_task = curr_task->next_task_pointer;
				task_count = task_count - 1;
				exited = 1;
			}			
			
			//If the next task to be selected is the one to be exited
			else if(curr_task->next_task_pointer == tcb){				
				//Change the current tasks next task pointer to the next next task pointer
				curr_task->next_task_pointer = curr_task->next_task_pointer->next_task_pointer;
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



//"wait" task callback
static void simpleRoundRobin_wait(void * const reason, uint32_t checkcode){
	
	
	if(checkcode == OS_getCheckCode()){	

		

		OS_TCB_t * curr_task_check = head_task;
		
		
		//Remove the current TCB from the ready task list then update its status		
		for(uint_fast8_t i = 0; i <task_count; i++){
			//If the task to be waited is the head task
			if(i == 0){
				if(OS_currentTCB() == curr_task_check){
							
					head_task = OS_currentTCB()->next_task_pointer;
					waiting_task_count = waiting_task_count + 1;
					break;
				}					
			}
			else{
				//If the next task is the task to be waited - remove it from the list
				if(curr_task_check->next_task_pointer == OS_currentTCB()){
					OS_TCB_t *task_to_remove = curr_task_check->next_task_pointer;
					
					//Set the current tasks next pointer to be the task after the task to be removed
					curr_task_check->next_task_pointer = task_to_remove->next_task_pointer;
					//Set the task to be removed pointer to be null
					task_to_remove->next_task_pointer = NULL;
					
					waiting_task_count = waiting_task_count + 1;
					
					break;
				}		
				
				//If the code gets here something bad has happened!!!!!
				//There is no matching task in the task list that is going to be waited
				
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
static void simpleRoundRobin_notify(OS_TCB_t * task_to_notify){

	//Update the tasks status flags
	task_to_notify->data = 0;
	
	task_to_notify->state &= ~TASK_STATE_WAITING;
	
	//Put the task back in the ready-task linked list
	OS_TCB_t * curr_task = head_task;
	
	//If there are no active tasks
	if (curr_task == NULL){	
		head_task = task_to_notify;
		waiting_task_count = waiting_task_count - 1;
		return;
	}
	
	//Finds the next slot in the linked list 
	while(curr_task->next_task_pointer != NULL){
		curr_task = curr_task->next_task_pointer;	
	}
	
	curr_task->next_task_pointer = task_to_notify;
	waiting_task_count = waiting_task_count - 1;
	
}




