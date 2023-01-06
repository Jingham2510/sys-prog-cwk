#ifndef SLEEP_H
#define SLEEP_H

#include <stdint.h>
#include "task.h"


//Requests that the task be idled for the specified number of ticks
//Also tells the OS to remove the task from the active list
//Note: Done outside of the SVC handler, so that OS_yeild can be called correctly
void OS_sleephandler(int32_t const sleep_ticks);


#endif /* SLEEP_H */
