#include "mempool.h"



//Initialise the pool and its mutex
void OS_pool_init(OS_mempool_t * const pool){	

	pool->head = 0;
	
	OS_mutex_init(&pool->mutex);

}


//Allocate a block of memory to a variable that calls this function
void * OS_pool_allocate(OS_mempool_t * const pool){

		//Before the task can claim memory, it needs to get the mutex
		OS_mutex_acquire(&pool->mutex);
		
		//Store the address of the currently free memory
		void *allocated_memory = pool->head;
		
		//Update the pool head to point to he next free block of memory (which is pointed to by the head of the pool)
		pool->head = * (void * *) pool->head;
		
		//Free the mutex now that the memory pool has been edited
		OS_mutex_release(&pool->mutex);
		
		//Return the block of memory beign allocated
		return allocated_memory;
		
}


//Take the memory form a given variable and place it in the memory pool
void OS_pool_deallocate(OS_mempool_t * const pool, void * const item){

	//Acquire the pools mutex so before the pool can be edited
	OS_mutex_acquire(&pool->mutex);	
	
	//Make the memory location of the item the new memory pool head	
	//Point to the void pointer of the item (i.e. point to where the void pointer is stored)
	void * * item_pointer = (void * *) item;
	
	//Set the memory location to store the pointer to the current pool head
	* item_pointer = pool->head;
	
	//Set the pool head to point to the items memory
	pool->head = item;
	
	//Finished editing the pool, so free the mutex
	OS_mutex_release(&pool->mutex);

}





