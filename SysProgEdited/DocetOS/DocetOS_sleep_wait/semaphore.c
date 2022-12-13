#include "semaphore.h"
#include "stm32f4xx.h"
#include "os.h"
#include <stddef.h>

//Initialies the semaphore structure
void OS_semaphore_init(OS_semaphore_t * semaphore, uint32_t start_count){

	semaphore->counter = start_count;
	
	semaphore->waiting_count = 0;

}


//Attempts to acquire a semaphore token from the semaphore pot
void OS_semaphore_acquire(OS_semaphore_t * semaphore){

	uint_fast8_t complete = 0;

	//The task attempts to acquire a token
	while (!complete){
	
		//Load the current number of tokens in the semaphore and check that it isnt empty
		uint32_t token_count =  __LDREXW(&(semaphore->counter));
		
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
			OS_notify(semaphore);
		}
	
	
	}
	
	





}
