#include "simpleRoundRobin.h"
#include <stm32f4xx.h>
#include "semaphore.h"



/* Prototypes (functions are static, so these aren't in the header file) */
static OS_TCB_t const * simpleRoundRobin_scheduler(void);
static void simpleRoundRobin_addTask(OS_TCB_t * const tcb);
static void simpleRoundRobin_taskExit(OS_TCB_t * const tcb);

static void simpleRoundRobin_wait(void * const reason, uint32_t checkcode);
static void simpleRoundRobin_notify(OS_TCB_t * const task_to_notify);

static void simpleRoundRobin_sleep(void);
static void simpleRoundRobin_wake(void);



//The task at the head of the active task linked list
static OS_TCB_t *head_active_task;


//The task at the head of the sleeping task linked list
static OS_TCB_t *head_sleeping_task;

//A count to keep track of the number of tasks
static uint32_t task_count = 0;
//A count to keep track of the number of waiting tasks
static uint32_t waiting_task_count = 0;
//A count to keep track of the number of sleeping tasks
static uint32_t sleeping_task_count = 0;

/* Scheduler block for the simple round-robin */
OS_Scheduler_t const simpleRoundRobinScheduler = {
	.preemptive = 1,
	.scheduler_callback = simpleRoundRobin_scheduler,
	.addtask_callback = simpleRoundRobin_addTask,
	.taskexit_callback = simpleRoundRobin_taskExit,
	.wait_callback = simpleRoundRobin_wait,
	.notify_callback = simpleRoundRobin_notify,
	.sleep_callback = simpleRoundRobin_sleep,
	.wake_callback = simpleRoundRobin_wake	
};


/* Round-robin scheduler callback */
static OS_TCB_t const * simpleRoundRobin_scheduler(void) {
	
	//Get the current TCB	
	OS_TCB_t * curr_task = OS_currentTCB();
	

	//Check to see if there are sleeping tasks
	if(head_sleeping_task != NULL){
		OS_wake();
	}				
	//Check to see if any task is available to run
	uint32_t active_tasks = task_count - waiting_task_count - sleeping_task_count;
	if (active_tasks > 0){
		
		//If their are active_tasks and we are idling, stop idling
		if(curr_task == OS_idleTCB_p){
			return head_active_task;
		}else{		
			return curr_task->next_task_pointer;
		}			
	}
	// No active tasks, so return the idle task
	else{
		//printf("IDLING");
		return OS_idleTCB_p;
	}
}

/* 'Add task' callback */
static void simpleRoundRobin_addTask(OS_TCB_t * const tcb) {

	if(task_count < SIMPLE_RR_MAX_TASKS){		
		//Start the search process at the head task
		OS_TCB_t * curr_task = head_active_task;		
		
		//If there is no head task
		if(head_active_task == NULL){
			head_active_task = tcb;
			task_count = task_count + 1;			
		}else{	
			while(curr_task->next_task_pointer != head_active_task){		
					curr_task = curr_task->next_task_pointer;
			}			
			curr_task->next_task_pointer = tcb;
			task_count = task_count + 1;
		}
		
		//Make it a circular linked list
		tcb->next_task_pointer = head_active_task;		
		}
		
	// If we get here, there are no free TCB slots, so the task just won't be added
	return;
}

/* 'Task exit' callback */
static void simpleRoundRobin_taskExit(OS_TCB_t * const tcb) {
	// Remove the given TCB from the list of tasks so it won't be run again
	//Boolean to keep track of when the task has been removed from the link list
	uint_fast8_t exited = 0;
	
	OS_TCB_t * curr_task = head_active_task;
	
	while(!exited){			
				
			//if the task to be exited is the head task
			if(head_active_task  == tcb){					
				
				//Make sure that the end of the list now points to the new head
				while(curr_task->next_task_pointer != head_active_task){
					curr_task = curr_task->next_task_pointer;					
				}				
				curr_task->next_task_pointer = tcb->next_task_pointer;
				
				//Place the head at the head of the list
				head_active_task = tcb->next_task_pointer;			
				
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
				curr_task = curr_task->next_task_pointer;
				
				//Check to make sure that we haven't looped back to the start
				if(curr_task == head_active_task){
					return;
				}			
			}	
	}
	
	//Clear the tcbs next task (as the tcb no longer in the list)
	tcb->next_task_pointer = NULL;
	

	

}



//"wait" task callback
static void simpleRoundRobin_wait(void * const reason, uint32_t checkcode){
	
	if(checkcode == OS_getCheckCode()){		

		
		//Get the task to be waited
		OS_TCB_t * task_to_wait = OS_currentTCB();		
		
		
		
		//Remove the current TCB from the ready task list then update its status			
		//Find the task to be removed in the active list
		//If the task to be removed is the first task
		if(task_to_wait == head_active_task){						
				
			//If its the only task in the queue -- Clear the queue
				if(head_active_task == head_active_task->next_task_pointer){
					head_active_task = NULL;
				}
				else{				
					head_active_task =  head_active_task->next_task_pointer;
				}	
			}		
		else{			
			OS_TCB_t * curr_task_check = head_active_task;
			
			while(curr_task_check->next_task_pointer != task_to_wait){
				curr_task_check = curr_task_check->next_task_pointer;
			}			
			curr_task_check->next_task_pointer = curr_task_check->next_task_pointer->next_task_pointer;				
		}		
		
		task_to_wait->next_task_pointer = NULL;		
		
		waiting_task_count = waiting_task_count + 1;
		
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
	OS_TCB_t * curr_task = head_active_task;
	
	//If there are no active tasks
	if (curr_task == NULL){	
		head_active_task = task_to_notify;
		waiting_task_count = waiting_task_count - 1;
		task_to_notify->next_task_pointer = head_active_task;
		return;
	}
	
	//Finds the next slot in the linked list 
	while(curr_task->next_task_pointer != NULL){
		curr_task = curr_task->next_task_pointer;	
	}
	
	curr_task->next_task_pointer = task_to_notify;
	task_to_notify->next_task_pointer = head_active_task;
	waiting_task_count = waiting_task_count - 1;
	
}



//"Sleep" task callback
//Removes the current task from the active list and places it in the sleeping list
static void simpleRoundRobin_sleep(){
	
	//Get the task to be slept
	OS_TCB_t * task_to_sleep = OS_currentTCB();	
	
	
	//Get the start of the active task linked list
	OS_TCB_t * curr_task_check = head_active_task;
	
	
			//Remove the current TCB from the ready task list then update its status			
		//Find the task to be removed in the active list
		//If the task to be removed is the first task
		if(task_to_sleep == head_active_task){						
				
			//If its the only task in the queue -- Clear the queue
				if(head_active_task == head_active_task->next_task_pointer){
					head_active_task = NULL;
				}
				else{				
					head_active_task =  head_active_task->next_task_pointer;
				}
				
		
			}		
		else{			
			OS_TCB_t * curr_task_check = head_active_task;
			
			while(curr_task_check->next_task_pointer != task_to_sleep){
				curr_task_check = curr_task_check->next_task_pointer;
			}			
			curr_task_check->next_task_pointer = curr_task_check->next_task_pointer->next_task_pointer;				
		}
		
		task_to_sleep->next_task_pointer = NULL;
	
	
	
	
	//Increase the sleeping task count
	sleeping_task_count = sleeping_task_count + 1;	
	
	//Place the task in the sleeping list (ordered)	
	//If there are currently no sleeping tasks
	if(head_sleeping_task == NULL){		
		head_sleeping_task = task_to_sleep;
		
	}
	//If there are sleeping tasks
	else{
		
			uint_fast8_t placed = 0;
		
			OS_TCB_t * curr_sleep_check = head_sleeping_task;
			
		//If the time to be woken is nearer than the current time to be woken at the head
			if(task_to_sleep->data < curr_sleep_check->data){
				
				task_to_sleep->next_task_pointer = curr_sleep_check;
				
				curr_sleep_check  = task_to_sleep;
				
				placed = 1;
			}	
			
			while(!placed){
				//If the task is at the end of the list
				if(curr_sleep_check->next_task_pointer == NULL){
					curr_sleep_check->next_task_pointer = task_to_sleep;
					placed = 1;
				}
				else{					
					//If the task is going to be woken sooner than the next task
					//Insert it between the tasks
					if(task_to_sleep->data < curr_sleep_check->next_task_pointer->data){
						
						task_to_sleep->next_task_pointer = curr_sleep_check->next_task_pointer;
						
						curr_sleep_check->next_task_pointer = task_to_sleep;
						
						placed = 1;
						
					}
					else{
						curr_sleep_check = curr_sleep_check->next_task_pointer;
					}				
				}				
			}
	}
	
	
	
}


//"Wake" task callback
static void simpleRoundRobin_wake(){	
	//Check the first task in the sleeping list to see if enough time has elapsed
	if( (int32_t) (OS_elapsedTicks() - head_sleeping_task->data) > 0){

			OS_TCB_t * task_to_wake = head_sleeping_task;
		
		
			//Clear the yield flag and the data
			task_to_wake->state &= ~TASK_STATE_SLEEP;
			task_to_wake->data = 0;							

			// place the sleeping task in the active list
			OS_TCB_t * curr_task = head_active_task;
		
			if(curr_task == NULL){
				head_active_task = task_to_wake;				
			}
			else{
				while(curr_task->next_task_pointer != NULL){
					curr_task = curr_task->next_task_pointer;
				}
				curr_task->next_task_pointer = task_to_wake;				
			}
			
			//remove the sleeping task from the sleeping list
			head_sleeping_task = task_to_wake->next_task_pointer;
			
			task_to_wake->next_task_pointer = head_active_task;			

			//Reduce the sleeping task list count
			sleeping_task_count = sleeping_task_count -1;

			//We can only wake one task at a time but thats okay as this is called every scheduler runthrough
	
	}
	else{
		//If it hasn't we dont need to check the rest of the list because its ordered
		return;
	}	
	
	
	
	
}



