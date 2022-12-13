#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <stddef.h>
#include "mutex.h"

/* Structure of the memory pool */
typedef struct mempool{
	//Head of the memory pool
	void *head;
	//Mutex which protects the pool while memory is being allocated/deallocated
	OS_mutex_t mutex; 
}OS_mempool_t;

//Initialises a memory pool structure
void pool_init(OS_mempool_t *pool);

//Allocates a section of the pools memory
void * pool_allocate(OS_mempool_t *pool);

//Frees up a given items memory block and returns it back into the memory pool
void pool_deallocate(OS_mempool_t *pool, void * item);



#define pool_add pool_deallocate






#endif /* MEMPOOL_H*/
