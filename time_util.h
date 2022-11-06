#ifndef __time_util_h__
#define __time_util_h__

#include <stdint.h>
#include <time.h>

void time_tick_1sec();
void set_time_tick(time_t new_timestamp);
char* get_ts_str();

#endif /* __time_util_h__ */