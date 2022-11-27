#ifndef __charger_logic_h__
#define __charger_logic_h__

//#include "r_smc_entry.h"
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float overall_highest_charge;
    float overall_highest_energy;
    bool car_active;
    bool kl15_pwr_state;
    bool line_pwr_state;
    bool heater_active_state;
    bool balancer_active_state;
    bool charger_active_state;
} charger_logic_globals_t;

void main_timer_tick(void);


void charger_logic_tick();
void cl_balancer_on();
void cl_balancer_off();
void cl_heater_on();
void cl_heater_off();
void print_charger_logic_status();
void freeze_charger_logic_status();
void update_charger_logic_timestamps(time_t timestamp_delta);

#endif /* __charger_logic_h__ */