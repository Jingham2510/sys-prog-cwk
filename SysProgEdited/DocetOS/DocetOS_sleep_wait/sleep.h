#ifndef SLEEP_H
#define SLEEP_H

#include <stdint.h>





//Requests that the task be idled for the specified number of ticks
void OS_sleep(int32_t sleep_ticks);


#endif /* SLEEP_H */
