#include "mutex.h"
#include "stm32f4xx.h"
#include "os.h"
#include <stddef.h>


//Initialies a mutex structure
void OS_mutex_init(OS_mutex_t * mutex){
	
	mutex->counter = 0;
	
  mutex->TCB_pointer = NULL;
	
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
			OS_wait(mutex, OS_getCheckCode());				
		}else{			
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
			OS_notify(mutex);					
		}
	}	
}
