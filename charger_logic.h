#ifndef __charger_logic_h__
#define __charger_logic_h__

//#include "r_smc_entry.h"
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#define USER_REQUEST_HEATER_ONLY_NO_CHARGE true
#define USER_REQUEST_DEFAULT false

#define REASON_CHARGE_NOT_STARTING_TEMP_LOW                 (1<<1)
#define REASON_CHARGE_NOT_STARTING_TEMP_HIGH                (1<<2)
#define REASON_CHARGE_NOT_STARTING_VOLT_CHG_NOT_NEEDED      (1<<3)
#define REASON_CHARGE_NOT_STARTING_CELL_DATA_TOO_OLD        (1<<4)
#define REASON_CHARGE_NOT_STARTING_USER_REQUEST             (1<<5)
#define REASON_CHARGE_NOT_STARTING_TOO_MANY_CHARGING_ATTEMPTS_SINCE_LINE_PWR    (1<<6)
#define REASON_CHARGE_NOT_STARTING_COOLDOWN_BETWEEN_TWO_CHARGING_ATTEMPTS       (1<<7)

#define REASON_BALANCER_NOT_STARTING_TEMP_LOW                 (1<<1)
#define REASON_BALANCER_NOT_STARTING_TEMP_HIGH                (1<<2)
#define REASON_BALANCER_NOT_STARTING_CELL_DATA_TOO_OLD        (1<<4)


typedef struct {
    float overall_highest_charge;
    float overall_highest_energy;
    bool car_active;
    bool kl15_pwr_state;
    bool line_pwr_state;
    bool heater_active_state;
    bool balancer_active_state;
    bool charger_active_state;
    bool user_request_heater_only_no_charge;
    uint8_t num_of_charging_attempts_since_line_pwr;
    uint8_t reason_charge_not_starting;
    uint8_t reason_balancer_not_starting;
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

void set_user_request(uint8_t user_request);

#endif /* __charger_logic_h__ */