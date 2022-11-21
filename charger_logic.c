#include "charger_logic.h"

#include <stdint.h>
#include "shunt.h"
#include "log_util.h"
#include "freeze_util.h"
#include "cellmodule_data.h"

#define LOG_AND_FREEZE(...)   freeze_va(__VA_ARGS__);capture_compact_freeze_frame=2;
#define HOURS_TILL_NOW(timestamp)   get_dur_full_hours(get_system_time() - timestamp )
#define MIN_TILL_NOW(timestamp)    get_dur_full_minutes(get_system_time() - timestamp )

extern shunt_t shunt_data;
extern uint8_t capture_compact_freeze_frame;

// true if charging (LINE DETECT) or key-on (KL15 ON)
static bool car_active = false;
static bool kl15_pwr_state = false;
static bool line_pwr_state = false;
static bool heater_active_state = false;
static bool balancer_active_state = false;
static bool charger_active_state = false;
static uint8_t reason_charge_not_starting = 0;
static uint8_t reason_balancer_not_starting = 0;
static time_t charge_started_ts = 0;
static time_t charge_ended_ts = 0;
static time_t kl15_started_ts = 0;
void charger_logic_tick()
{
    static float charge_started_charge = 0;
    static float charge_started_energy = 0;
    static float charge_ended_charge = 0;
    static float charge_ended_energy = 0;
    static float kl15_started_charge = 0;
    static float kl15_started_energy = 0;
/// ensure relais coils IDLE is reached
    OUT_BAL_LATCH_OFF_IDLE
    OUT_BAL_LATCH_ON_IDLE
    OUT_HEATER_LATCH_OFF_IDLE
    OUT_HEATER_LATCH_ON_IDLE
    bool check_age_ticks_u_batt_and_temp_allowed_var = check_age_ticks_u_batt_and_temp_allowed();
/// check line power active
    if (IN_SIGNAL_LINE_PWR)
    {
        // line power on
        if (!line_pwr_state)
        {
            // line power on latch
            LOG_AND_FREEZE("LINE DETECT LATCH ON\n");
            line_pwr_state = true;
        }
/// check need for heating
        bool check_temp_should_use_heater_var = check_temp_should_use_heater();
        // bool check_age_ticks_u_batt_and_temp_allowed_var = check_age_ticks_u_batt_and_temp_allowed();
        if (check_temp_should_use_heater_var && check_age_ticks_u_batt_and_temp_allowed_var)
        {
            if (!heater_active_state)
            {
                LOG_AND_FREEZE("HEATER LATCH ON\n");
                cl_heater_on();
            }
        } else {
            if (heater_active_state)
            {
                char* msg_heater = "";
                char* msg_tick_age = "";
                if(!check_temp_should_use_heater_var) {
                    msg_heater = ":REACHED TEMP";
                }
                if (!check_age_ticks_u_batt_and_temp_allowed_var) {
                    msg_tick_age = ":CELL DATA TO OLD";
                }

                LOG_AND_FREEZE("HEATER LATCH OFF%s%s\n", msg_heater, msg_tick_age);
                cl_heater_off();
            }
        }
/// check if good for charging
        bool check_temp_charging_allowed_var = check_temp_charging_allowed();
        bool check_volt_charging_necessary_start_var = check_volt_charging_necessary_start();
        //bool check_age_ticks_u_batt_and_temp_allowed_var = check_age_ticks_u_batt_and_temp_allowed();
        if (check_temp_charging_allowed_var && check_volt_charging_necessary_start_var && check_age_ticks_u_batt_and_temp_allowed_var)
        {
            if (!charger_active_state)
            {
                if (charge_ended_ts)
                {
                    // data of previous charge available
                    LOG_AND_FREEZE("CHARGE LATCH ON:LOAD ON expecting to recharge %.2Ah %.2kWh estimated duration %.1fh\n",
                        charge_ended_charge - shunt_data.charge, charge_ended_energy - shunt_data.energy, (charge_ended_charge - shunt_data.charge) / 20.0 * 1.2);
                }
                else
                {
                    // no data of previous charge available
                    LOG_AND_FREEZE("CHARGE LATCH ON:LOAD ON no data of prev charge avail\n");
                }
                OUT_CHARGER_LOAD_ON
                charger_active_state = true;
                reason_charge_not_starting = 0;
                if (shunt_report_charge_start())
                {
                    // shunt.charge and .energy will be reset to zero
                    charge_started_charge = 0;
                    charge_started_energy = 0;
                }
                else
                {
                    charge_started_charge = shunt_data.charge;
                    charge_started_energy = shunt_data.energy;
                }
                charge_started_ts = get_system_time();
                charge_ended_ts = 0;
                charge_ended_charge = 0;
                charge_ended_energy = 0;
            }
            else
            {
                OUT_CHARGER_DOOR_ON
            }
        } else {
            if (!charger_active_state)
            {
                char* msg_temp = "";
                char* msg_not_needed = "";
                char* msg_tick_age = "";
                uint8_t new_reason_charge_not_starting = 0;
                if(!check_temp_charging_allowed_var) {
                    msg_temp = ":TEMP DOES NOT ALLOW";
                    new_reason_charge_not_starting |= (1<<1);
                }
                if(!check_volt_charging_necessary_start_var) {
                    msg_not_needed = ":VOLT CHG NOT NEEDED";
                    new_reason_charge_not_starting |= (1<<2);
                }
                if(!check_age_ticks_u_batt_and_temp_allowed_var) {
                    msg_tick_age = ":CELL DATA TO OLD";
                    new_reason_charge_not_starting |= (1<<3);
                }
                if (new_reason_charge_not_starting != reason_charge_not_starting)
                {
                    LOG_AND_FREEZE("NOT READY TO CHG%s%s%s\n", msg_temp, msg_not_needed, msg_tick_age);
                    reason_charge_not_starting = new_reason_charge_not_starting;
                }
            }
        }
/// check under/over-temp and charge-stop-voltage
        bool check_temp_charging_safety_stop_var = check_temp_charging_safety_stop();
        bool check_volt_charging_safety_stop_var = check_volt_charging_safety_stop();
        //bool check_age_ticks_u_batt_and_temp_allowed_var = check_age_ticks_u_batt_and_temp_allowed();
        if (check_temp_charging_safety_stop_var || check_volt_charging_safety_stop_var || !check_age_ticks_u_batt_and_temp_allowed_var)
        {
            OUT_CHARGER_DOOR_OFF
            if (charger_active_state)
            {
                char* msg_temp = "";
                char* msg_safety_stop = "";
                char* msg_tick_age = "";
                if(check_temp_charging_safety_stop_var) {
                    msg_temp = ":TEMP DOES NOT ALLOW";
                }
                if (check_volt_charging_safety_stop_var) {
                    msg_safety_stop = ":VOLT SAFETY STOP";
                    freezeframe_cellmodule_full_debug();
                }
                if (!check_age_ticks_u_batt_and_temp_allowed_var) {
                    msg_tick_age = ":CELL DATA TO OLD";
                }
                LOG_AND_FREEZE("CHARGE LATCH OFF:DOOR OFF%s%s%s charged %.2fAh %.2kWh in %dh%02dm\n", msg_temp, msg_safety_stop, msg_tick_age,
                    shunt_data.charge - charge_started_charge, shunt_data.energy - charge_started_energy,
                    HOURS_TILL_NOW(charge_started_ts), MIN_TILL_NOW(charge_started_ts));
                charger_active_state = false;
                charge_started_ts = 0;
                charge_started_charge = 0;
                charge_started_energy = 0;
                charge_ended_ts = get_system_time();
                charge_ended_charge = shunt_data.charge;
                charge_ended_energy = shunt_data.energy;
                reason_charge_not_starting = 0;
            } else {
                OUT_CHARGER_LOAD_OFF
            }
        }

    } else {
        // no line power
        if (line_pwr_state)
        {
            // line power off latch
            line_pwr_state = false;
            LOG_AND_FREEZE("LINE DETECT LATCH OFF:HEATER OFF:CHG LOAD OFF:CHG DOOR OFF\n");
            charger_active_state = false;
            cl_heater_off();
            OUT_CHARGER_LOAD_OFF
            OUT_CHARGER_DOOR_OFF
            reason_charge_not_starting = 0;
        }
    }

/// check KL15 power active
    if (IN_SIGNAL_KL15_PWR)
    {
        // KL15 on
        if (!kl15_pwr_state)
        {
            // KL15 on latch
            if (charge_ended_ts)
            {
                // data of previous charge available
                LOG_AND_FREEZE("KL15 DETECT LATCH ON has avail %.2fAh %.2fkWh - last charging ended %dh%02dm ago\n", shunt_data.charge, shunt_data.energy,
                    HOURS_TILL_NOW(charge_ended_ts), MIN_TILL_NOW(charge_ended_ts));
            }
            else
            {
                // no data of previous charge available
                LOG_AND_FREEZE("KL15 DETECT LATCH ON has avail %.2fAh %.2fkWh - no data of prev charge avail\n", shunt_data.charge, shunt_data.energy);
            }
            kl15_pwr_state = true;
            kl15_started_ts = get_system_time();
            kl15_started_charge = shunt_data.charge;
            kl15_started_energy = shunt_data.energy;
        }
    } else {
        // no KL15
        if (kl15_pwr_state)
        {
            // KL15 off latch
            LOG_AND_FREEZE("KL15 DETECT LATCH OFF used %.2fAh %.2fkWh in %dh%02dm\n",
                kl15_started_charge - shunt_data.charge, kl15_started_energy - shunt_data.energy,
                HOURS_TILL_NOW(kl15_started_ts), MIN_TILL_NOW(kl15_started_ts));
            kl15_pwr_state = false;
            kl15_started_ts = 0;
            kl15_started_charge = 0;
            kl15_started_energy = 0;
        }
    }


/// check if car is sleeping or not
    if (kl15_pwr_state || line_pwr_state)
    {
        // car active (KL15 ON and / or LINE DETECT)
        if (!car_active)
        {
            LOG_AND_FREEZE("CAR ACTIVE LATCH ON:BAL ON\n");
            car_active = true;
        }
    } else {
        // car sleeping
        if (car_active)
        {
            LOG_AND_FREEZE("CAR ACTIVE LATCH OFF\n");
            car_active = false;
        }
    }

/// check if balancer is needed and allowed
    bool check_temp_balancing_allowed_var = check_temp_balancing_allowed();
    if (car_active)
    {
        if (!balancer_active_state)
        {
            // balancer not active
            if (check_temp_balancing_allowed_var && check_age_ticks_u_batt_and_temp_allowed_var)
            {
                cl_balancer_on();
            }
            else
            {
                char* msg_temp = "";
                char* msg_tick_age = "";
                uint8_t new_reason_balancer_not_starting = 0;
                if(!check_temp_balancing_allowed_var) {
                    msg_temp = ":TEMP DOES NOT ALLOW";
                    new_reason_balancer_not_starting |= (1<<1);
                }
                if(!check_age_ticks_u_batt_and_temp_allowed_var) {
                    msg_tick_age = ":CELL DATA TO OLD";
                    new_reason_balancer_not_starting |= (1<<2);
                }
                if (new_reason_balancer_not_starting != reason_balancer_not_starting)
                {
                    LOG_AND_FREEZE("BAL NOT ON%s%s%s\n", msg_temp, msg_tick_age);
                    reason_balancer_not_starting = new_reason_balancer_not_starting;
                }
            }
        }
        else
        {
            // balancer active
            if (!check_temp_balancing_allowed_var || !check_age_ticks_u_batt_and_temp_allowed_var)
            {
                cl_balancer_off();
                char* msg_temp = "";
                char* msg_tick_age = "";
                if(!check_temp_balancing_allowed_var) {
                    msg_temp = ":TEMP DOES NOT ALLOW";
                }
                if(!check_age_ticks_u_batt_and_temp_allowed_var) {
                    msg_tick_age = ":CELL DATA TO OLD";
                }
                LOG_AND_FREEZE("BAL OFF (CAR active)%s%s\n", msg_temp, msg_tick_age);
            }
        }
    }
    else
    {
        // car not active
        if (balancer_active_state && (!check_temp_balancing_allowed_var || !check_age_ticks_u_batt_and_temp_allowed_var))
        {
            cl_balancer_off();
            char* msg_temp = "";
                char* msg_tick_age = "";
                if(!check_temp_balancing_allowed_var) {
                    msg_temp = ":TEMP DOES NOT ALLOW";
                }
                if(!check_age_ticks_u_batt_and_temp_allowed_var) {
                    msg_tick_age = ":CELL DATA TO OLD";
                }
                LOG_AND_FREEZE("BAL OFF (CAR not active)%s%s\n", msg_temp, msg_tick_age);
        }
        reason_balancer_not_starting = 0;
    }
}



void cl_balancer_on()
{
    LOG_AND_FREEZE("BAL LATCH ON\n");
    balancer_active_state = true;
    reason_balancer_not_starting = 0;
    OUT_BAL_LATCH_OFF_IDLE
    OUT_BAL_LATCH_ON_CURR
}
void cl_balancer_off()
{
    LOG_AND_FREEZE("BAL LATCH OFF\n");
    balancer_active_state = false;
    reason_balancer_not_starting = 0;
    OUT_BAL_LATCH_ON_IDLE
    OUT_BAL_LATCH_OFF_CURR
}
void cl_heater_on()
{
    LOG_AND_FREEZE("HEATER LATCH ON\n");
    heater_active_state = true;
    OUT_HEATER_LATCH_OFF_IDLE
    OUT_HEATER_LATCH_ON_CURR
}
void cl_heater_off()
{
    LOG_AND_FREEZE("HEATER LATCH OFF\n");
    heater_active_state = false;
    OUT_HEATER_LATCH_ON_IDLE
    OUT_HEATER_LATCH_OFF_CURR
}
void print_charger_logic_status()
{
    #pragma diag_suppress=Pa082
    log_va("[CL: CAR:%d KL15:%d LINE:%d BAL:%d HEATER:%d | CHARGE_ON:%d CHG_DOOR:%d CHG_LOAD:%d REASON:%02X\n", car_active, kl15_pwr_state, line_pwr_state, balancer_active_state, heater_active_state, charger_active_state, IS_OUT_CHARGER_DOOR_ON, IS_OUT_CHARGER_LOAD_ON, reason_charge_not_starting);
}
void freeze_charger_logic_status()
{
    #pragma diag_suppress=Pa082
    freeze_va("[CL: CAR:%d KL15:%d LINE:%d BAL:%d HEATER:%d | CHARGE_ON:%d CHG_DOOR:%d CHG_LOAD:%d REASON:%02X\n", car_active, kl15_pwr_state, line_pwr_state, balancer_active_state, heater_active_state, charger_active_state, IS_OUT_CHARGER_DOOR_ON, IS_OUT_CHARGER_LOAD_ON, reason_charge_not_starting);
}



void update_charger_logic_timestamps(time_t timestamp_delta)
{
    // only update timestamps, if already set (not zero)
    if (charge_started_ts) { charge_started_ts += timestamp_delta; }
    if (charge_ended_ts) { charge_ended_ts += timestamp_delta; }
    if (kl15_started_ts) { kl15_started_ts += timestamp_delta; }
}