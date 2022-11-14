#include "time_util.h"

#include "log_util.h"

// 01.11.2022 00:00:00 (UTC)
#define START_TIME 1667260800

static time_t system_time_sec = START_TIME;

void time_tick_1sec()
{
    system_time_sec++;
}

time_t get_system_time()
{
    return system_time_sec;
}

void set_time_tick(time_t new_timestamp)
{
    if (new_timestamp >= START_TIME)
    {
        char buf_prev[20] = {0};
        char buf_new[20] = {0};
        strftime(buf_prev, 20, "%y%m%d %H%M%S", gmtime(&system_time_sec));
        strftime(buf_new, 20, "%y%m%d %H%M%S", gmtime(&new_timestamp));
        log_va("prev-ts: %s new-ts: %s\n", buf_prev, buf_new);
        system_time_sec = new_timestamp;
    }
}

char* get_ts_str() // 12 chars + '\0'
{
    static char buf[20] = {0};
    strftime(buf, 20, "%m%d %H%M%S ", gmtime(&system_time_sec));
    return buf;
}

char* get_ts_full_str()
{
    static char buf[20] = {0};
    strftime(buf, 20, "%y%m%d %H%M%S", gmtime(&system_time_sec));
    return buf;
}

uint16_t get_dur_full_hours(time_t duration)
{
    return duration/3600;
}

uint8_t get_dur_full_minutes(time_t duration)
{
    return (duration/60)%60;
}