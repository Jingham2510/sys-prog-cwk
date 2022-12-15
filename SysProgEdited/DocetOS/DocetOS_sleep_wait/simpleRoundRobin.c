#include "simpleRoundRobin.h"
#include <stm32f4xx.h>
#include "semaphore.h"

/* This is an implementation of an extremely simple (and inefficient!) round-robin scheduler.

   An array of pointers to TCBs is declared, and when tasks are added they are inserted into
	 this array.  When tasks finish, they are removed from the array (pointers are set back to
	 zero).  When the scheduler is invoked, it simply looks for the next available non-zero
	 pointer in the array, and returns it.  If there are no non-zero TCB pointers in the array,
	 a pointer to the idle task is returned instead.
	 
	 The inefficiency in this implementation arises mainly from the way arrays are used for list
	 storage.  If the array is large and the number of tasks is small, the scheduler will spend
	 a lot of time looking through the array each time it is called. */

/* Prototypes (functions are static, so these aren't in the header file) */
static OS_TCB_t const * simpleRoundRobin_scheduler(void);
static void simpleRoundRobin_addTask(OS_TCB_t * const tcb);
static void simpleRoundRobin_taskExit(OS_TCB_t * const tcb);

static void simpleRoundRobin_wait(void * const reason, uint32_t checkcode);
static void simpleRoundRobin_notify(void * const reason);

/* REMOVE THIS ONCE LINKED LIST IMPLEMENTED AND WORKS */
static OS_TCB_t * tasks[SIMPLE_RR_MAX_TASKS] = {0};

//The task at the head of the task linked list -if none, set to idle
static OS_TCB_t *head_task;

//A count to keep track of the number of tasks - starts at 0
static uint32_t task_count = 0;

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
	
	
	

	// No tasks in the list, so return the idle task
	return OS_idleTCB_p;
}

/* 'Add task' callback */
static void simpleRoundRobin_addTask(OS_TCB_t * const tcb) {
	if(task_count == 0){
		head_task = tcb;
		task_count = task_count + 1;
	}
	else if(task_count < 8){
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

		
		OS_currentTCB()->data = (uint32_t) reason;
						
		//Set the waiting flag high
		OS_currentTCB()->state |= TASK_STATE_WAITING;
		
		//Invoke the scheduler to context switch
		SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
		
	}	
	
}


//"notify" task callback
static void simpleRoundRobin_notify(void * const reason){

	static int i = 0;
	
	//Loop through every task in the list
	for (int j = 1; j <= SIMPLE_RR_MAX_TASKS; j++) {
		i = (i + 1) % SIMPLE_RR_MAX_TASKS;
		
		//If the task is waiting and the reasons match - clear the waiting flag and the data
		if((tasks[i]->state & TASK_STATE_WAITING) && (tasks[i]->data  == (uint32_t) reason)){
			
			tasks[i]->data = 0;
			tasks[i]->state &= ~TASK_STATE_WAITING;	
	


			
					
		}		
	}
	

	
}




