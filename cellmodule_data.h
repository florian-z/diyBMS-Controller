#ifndef __cellmodule_data_h__
#define __cellmodule_data_h__

#include <stdbool.h>
#include "main.h"
#include "r_cg_userdefine.h"



typedef struct {
    uint16_t u_batt_mv;
    int8_t temp_batt_c;
    int8_t temp_aux_c;
} module_data_t;

typedef struct {
    uint16_t u_batt;
    uint16_t temp;
} module_data_age_t;

typedef struct {
    uint16_t u_batt_mv_lowest;
    uint16_t u_batt_mv_mean;
    uint16_t u_batt_mv_highest;

    int8_t temp_batt_c_lowest;
    int8_t temp_batt_c_mean;
    int8_t temp_batt_c_highest;

    int8_t temp_aux_c_lowest;
    int8_t temp_aux_c_mean;
    int8_t temp_aux_c_highest;

    int8_t temp_shunt_c;

    int8_t temp_case_bottom_c_lowest;
    int8_t temp_case_bottom_c_mean;
    int8_t temp_case_bottom_c_highest;
} module_data_statistics_t;


#define BOTTOM_CASE_AUX_TEMP_ID_LEN 4
#define BOTTOM_CASE_AUX_TEMP_ID_1  1
#define BOTTOM_CASE_AUX_TEMP_ID_2  9
#define BOTTOM_CASE_AUX_TEMP_ID_3  37
#define BOTTOM_CASE_AUX_TEMP_ID_4  45
#define SHUNT_AUX_TEMP_ID 2
#define SHUNT_AUX_TEMP module_data[SHUNT_AUX_TEMP_ID].temp_aux_c

/// measurement values
void calc_cellmodule_data();
bool check_temp_should_use_heater();
bool check_temp_should_turn_off_heater();
bool check_temp_low_balancing_allowed();
bool check_temp_high_balancing_allowed();
bool check_temp_low_balancing_safety_stop();
bool check_temp_high_balancing_safety_stop();
bool check_temp_low_charging_allowed();
bool check_temp_high_charging_allowed();
bool check_volt_charging_necessary_start();
bool check_temp_low_charging_safety_stop();
bool check_temp_high_charging_safety_stop();
bool check_volt_charging_safety_stop();

/// age of cell module data
void tick_cellmodule();
uint16_t get_age_ticks_u_batt();
uint16_t get_age_ticks_temp();
bool check_age_ticks_u_batt_and_temp_allowed();

void log_cellmodule_full_debug();
void freezeframe_cellmodule_full_debug();
void freezeframe_cellmodule_compact_debug();

#endif /* __cellmodule_data_h__ */