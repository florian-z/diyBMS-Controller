#ifndef __user_main_h__
#define __user_main_h__

#include "r_smc_entry.h"

enum system_status {
    CELLMODULE_CHAIN1,
    CELLMODULE_CHAIN2,
    SHUNT,
    MAIN_LOOP
};

void main_timer_tick(void);

void Error_Handler(void);
void report_system_status(enum system_status);

#endif /* __main_h__ */