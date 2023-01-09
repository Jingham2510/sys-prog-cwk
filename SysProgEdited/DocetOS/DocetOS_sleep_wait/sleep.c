#include "sleep.h"
#include "os_internal.h"

//Requests that the task be idled for the specified number of ticks
void OS_sleep(int32_t const sleep_ticks){
	
	//Stores the time that the task wishes to be awoken in the TCB data
	OS_currentTCB()->data = OS_elapsedTicks() + sleep_ticks;
	
	//Set the state yield flag high
	OS_currentTCB()->state |= TASK_STATE_SLEEP; 
	
	//Initiate a task switch
	OS_yield();
	
}
