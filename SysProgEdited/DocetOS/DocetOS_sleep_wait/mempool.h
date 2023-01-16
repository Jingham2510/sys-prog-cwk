#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <stddef.h>
#include "mutex.h"






/*Data packet that could be stored in the memory pool*/
typedef struct {
	uint32_t id;
	
	uint32_t data;
	
}mempool_datapacket_t;



/* Structure of the memory pool */
typedef struct mempool{
	//Head of the memory pool
	void * head;
	//Mutex which protects the pool while memory is being allocated/deallocated
	OS_mutex_t mutex; 
}OS_mempool_t;

//Initialises a memory pool structure
void OS_pool_init(OS_mempool_t * const pool);

//Allocates a section of the pools memory
void * OS_pool_allocate(OS_mempool_t * const pool);

//Frees up a given items memory block and returns it back into the memory pool
void OS_pool_deallocate(OS_mempool_t *const pool, void * const item);


//Macro for adding memory to a memory pool
#define OS_pool_add OS_pool_deallocate






#endif /* MEMPOOL_H*/
