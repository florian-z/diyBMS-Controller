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

// charger logic
void cl_balancer_on();
void cl_balancer_off();
void cl_heater_on();
void cl_heater_off();
void print_charger_logic_status();
void freeze_charger_logic_status();

void Error_Handler(void);
void report_system_status(enum system_status);

#endif /* __main_h__ */