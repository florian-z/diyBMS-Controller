#ifndef __time_util_h__
#define __time_util_h__

#include <stdint.h>
#include <time.h>

void time_tick_1sec();
time_t get_system_time();
void set_time_tick(time_t new_timestamp);
char* get_ts_str();
char* get_ts_full_str();

uint16_t get_dur_full_days(time_t duration);
uint16_t get_dur_full_hours(time_t duration);
uint8_t get_dur_full_minutes(time_t duration);

#endif /* __time_util_h__ */