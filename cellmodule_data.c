#include "cellmodule_data.h"

#include "log_util.h"
#include "freeze_util.h"
#include "uart_cellmodule.h"

// measurement of all cell modules
module_data_t module_data[MODULE_DATA_LEN] = {0};
// age of measurement data of both cellmodule-chains in ticks
module_data_age_t module_data_age[CELLMODULE_CHANNELS] = {{.u_batt=UINT16_MAX, .temp=UINT16_MAX},{.u_batt=UINT16_MAX, .temp=UINT16_MAX}};
// min, max, mean, etc. values of module_data
module_data_statistics_t module_data_stat = {0};


static const uint8_t BOTTOM_CASE_AUX_TEMP_IDS[BOTTOM_CASE_AUX_TEMP_ID_LEN] = { BOTTOM_CASE_AUX_TEMP_1, BOTTOM_CASE_AUX_TEMP_2, BOTTOM_CASE_AUX_TEMP_3, BOTTOM_CASE_AUX_TEMP_4 };


/// measurement values
/* calculate statistics */
void calc_cellmodule_data()
{
    {
        uint16_t tmp_low = UINT16_MAX;
        uint16_t tmp_high = 0;
        uint32_t tmp_sum = 0;
        for(uint8_t i=CELLMODULES_FIRST; i<MODULE_DATA_LEN; i++)
        {
            if (module_data[i].u_batt_mv < tmp_low) {
                tmp_low = module_data[i].u_batt_mv;
            }
            if (module_data[i].u_batt_mv > tmp_high) {
                tmp_high = module_data[i].u_batt_mv;
            }
            tmp_sum += module_data[i].u_batt_mv;
        }
        module_data_stat.u_batt_mv_lowest = tmp_low;
        module_data_stat.u_batt_mv_highest = tmp_high;
        module_data_stat.u_batt_mv_mean = (uint16_t)(tmp_sum / (float)CELLMODULES_TOTAL);
    }
    {
        int8_t tmp_low = INT8_MAX;
        int8_t tmp_high = INT8_MIN;
        int16_t tmp_sum = 0;
        for(uint8_t i=CELLMODULES_FIRST; i<MODULE_DATA_LEN; i++)
        {
            if (module_data[i].temp_batt_c < tmp_low) {
                tmp_low = module_data[i].temp_batt_c;
            }
            if (module_data[i].temp_batt_c > tmp_high) {
                tmp_high = module_data[i].temp_batt_c;
            }
            tmp_sum += module_data[i].temp_batt_c;
        }
        module_data_stat.temp_batt_c_lowest = tmp_low;
        module_data_stat.temp_batt_c_highest = tmp_high;
        module_data_stat.temp_batt_c_mean = (uint8_t)(tmp_sum / (float)CELLMODULES_TOTAL);
    }
    {
        int8_t tmp_low = INT8_MAX;
        int8_t tmp_high = INT8_MIN;
        int16_t tmp_sum = 0;
        for(uint8_t i=CELLMODULES_FIRST; i<MODULE_DATA_LEN; i++)
        {
            if (SHUNT_AUX_TEMP == i)
            {
                // skip shunt temp sensor. only use batt and case temp as temp_aux
                continue;
            }
            if (module_data[i].temp_aux_c < tmp_low) {
                tmp_low = module_data[i].temp_aux_c;
            }
            if (module_data[i].temp_aux_c > tmp_high) {
                tmp_high = module_data[i].temp_aux_c;
            }
            tmp_sum += module_data[i].temp_aux_c;
        }
        module_data_stat.temp_aux_c_lowest = tmp_low;
        module_data_stat.temp_aux_c_highest = tmp_high;
        module_data_stat.temp_aux_c_mean = (uint8_t)(tmp_sum / (float)CELLMODULES_TOTAL);
    }
    {
        int8_t tmp_low = INT8_MAX;
        int8_t tmp_high = INT8_MIN;
        int16_t tmp_sum = 0;
        for(uint8_t i=0; i<BOTTOM_CASE_AUX_TEMP_ID_LEN; i++)
        {
            if (module_data[BOTTOM_CASE_AUX_TEMP_IDS[i]].temp_aux_c < tmp_low) {
                tmp_low = module_data[BOTTOM_CASE_AUX_TEMP_IDS[i]].temp_aux_c;
            }
            if (module_data[BOTTOM_CASE_AUX_TEMP_IDS[i]].temp_aux_c > tmp_high) {
                tmp_high = module_data[BOTTOM_CASE_AUX_TEMP_IDS[i]].temp_aux_c;
            }
            tmp_sum += module_data[BOTTOM_CASE_AUX_TEMP_IDS[i]].temp_aux_c;
        }
        module_data_stat.temp_case_bottom_c_lowest = tmp_low;
        module_data_stat.temp_case_bottom_c_highest = tmp_high;
        module_data_stat.temp_case_bottom_c_mean = (uint8_t)(tmp_sum / (float)BOTTOM_CASE_AUX_TEMP_ID_LEN);
    }
    {
        module_data_stat.temp_shunt_c = module_data[SHUNT_AUX_TEMP].temp_aux_c;
    }
}

// use heater until all cells above LOW and none above HIGH
// true -> heater on
#define LIMITS_TEMP_HEATER_NEEDED_LOW 20
#define LIMITS_TEMP_HEATER_SAFETY_HIGH 35
bool check_temp_should_use_heater()
{
    if((module_data_stat.temp_batt_c_lowest < LIMITS_TEMP_HEATER_NEEDED_LOW || module_data_stat.temp_aux_c_lowest < LIMITS_TEMP_HEATER_NEEDED_LOW)
        && (module_data_stat.temp_batt_c_highest < LIMITS_TEMP_HEATER_SAFETY_HIGH && module_data_stat.temp_aux_c_highest < LIMITS_TEMP_HEATER_SAFETY_HIGH))
    {
        return true;
    }
    return false;
}

// charge only if all cells above LOW and none above HIGH
// true -> charging on
#define LIMITS_TEMP_CHARGING_NEEDED_LOW 15
#define LIMITS_TEMP_CHARGING_SAFETY_HIGH 45
bool check_temp_charging_allowed()
{
    if((module_data_stat.temp_batt_c_lowest > LIMITS_TEMP_CHARGING_NEEDED_LOW && module_data_stat.temp_aux_c_lowest > LIMITS_TEMP_CHARGING_NEEDED_LOW)
        && (module_data_stat.temp_batt_c_highest < LIMITS_TEMP_CHARGING_SAFETY_HIGH && module_data_stat.temp_aux_c_highest < LIMITS_TEMP_CHARGING_SAFETY_HIGH))
    {
        return true;
    }
    return false;
}

// start charging only if all cells below LOW and none above HIGH
// true -> charging on
#define LIMITS_VOLT_CHARGING_NECESSARY_LOW 3400
#define LIMITS_VOLT_CHARGING_NECESSARY_HIGH 3450
bool check_volt_charging_necessary_start()
{
    if(module_data_stat.u_batt_mv_lowest < LIMITS_VOLT_CHARGING_NECESSARY_LOW
        && module_data_stat.u_batt_mv_highest < LIMITS_VOLT_CHARGING_NECESSARY_HIGH)
    {
        return true;
    }
    return false;
}

// stop charging if one cell above HIGH
// true -> stop/end charging
#define LIMITS_VOLT_CHARGING_SAFETY_HIGH 3500
bool check_volt_charging_safety_stop()
{
    if(module_data_stat.u_batt_mv_highest >= LIMITS_VOLT_CHARGING_SAFETY_HIGH)
    {
        return true;
    }
    return false;
}



/// age of cell module data
/* cellmodule tick */
void tick_cellmodule()
{
    /* maintain cellmodule data age */
    for(uint8_t chain_id = 0; chain_id < CELLMODULE_CHANNELS; chain_id++)
    {
        if (module_data_age[chain_id].u_batt < UINT16_MAX) // prevent rollover
        {
            module_data_age[chain_id].u_batt++;
        }
        if (module_data_age[chain_id].temp < UINT16_MAX) // prevent rollover
        {
            module_data_age[chain_id].temp++;
        }
    }
}

/* get highest/oldest cellmodule u_batt-value age */
uint16_t get_age_ticks_u_batt()
{
    return ((module_data_age[CELL_MODULE_CHAIN_1].u_batt > module_data_age[CELL_MODULE_CHAIN_2].u_batt) ?
        module_data_age[CELL_MODULE_CHAIN_1].u_batt : module_data_age[CELL_MODULE_CHAIN_2].u_batt);
}

/* get highest/oldest cellmodule temp-value age */
uint16_t get_age_ticks_temp()
{
    return ((module_data_age[CELL_MODULE_CHAIN_1].temp > module_data_age[CELL_MODULE_CHAIN_2].temp) ?
        module_data_age[CELL_MODULE_CHAIN_1].temp : module_data_age[CELL_MODULE_CHAIN_2].temp);
}

// charge only if measurement data is not too old
// true -> charging allowed
#define LIMITS_TICKS_U_BATT_MAX_AGE 45
#define LIMITS_TICKS_TEMP_MAX_AGE 100
bool check_age_ticks_u_batt_and_temp_allowed()
{
    if (get_age_ticks_u_batt() < LIMITS_TICKS_U_BATT_MAX_AGE && get_age_ticks_temp() < LIMITS_TICKS_TEMP_MAX_AGE)
    {
        return true;
    }
    return false;
}



/// debug log
/* print all cellmodule data values and ages */
void log_cellmodule_full_debug()
{
    // measured values per module
    for(uint8_t i=1; i<=CELLMODULES_TOTAL; i++)
    {
        log_va("[%02d:%dmV:%d:%dC]%s", i, module_data[i].u_batt_mv, module_data[i].temp_batt_c, module_data[i].temp_aux_c, (!(i%10)?"\n":""));
    }
    log_va("\n");
    // age of measured values
    log_va("[age: ubatt %u temp %u]", get_age_ticks_u_batt(), get_age_ticks_temp());
    // calculated values
    log_va("[%u %u %u mV] [%d %d %d battC] [%d %d %d auxC] [%d %d %d caseBotC] [%d shuntC]\n",
        module_data_stat.u_batt_mv_lowest, module_data_stat.u_batt_mv_mean, module_data_stat.u_batt_mv_highest,
        module_data_stat.temp_batt_c_lowest, module_data_stat.temp_batt_c_mean, module_data_stat.temp_batt_c_highest,
        module_data_stat.temp_aux_c_lowest, module_data_stat.temp_aux_c_mean, module_data_stat.temp_aux_c_highest,
        module_data_stat.temp_case_bottom_c_lowest, module_data_stat.temp_case_bottom_c_mean, module_data_stat.temp_case_bottom_c_highest,
        module_data_stat.temp_shunt_c);
}
/* store all cellmodule data values and ages */
void freezeframe_cellmodule_full_debug()
{
    // measured values per module
    uint8_t i = 1;
    freeze_va("[%02d:%dmV:%d:%dC]", i, module_data[i].u_batt_mv, module_data[i].temp_batt_c, module_data[i].temp_aux_c);
    for(i++; i<=CELLMODULES_TOTAL; i++)
    {
        freeze_va_no_ts("[%02d:%dmV:%d:%dC]%s", i, module_data[i].u_batt_mv, module_data[i].temp_batt_c, module_data[i].temp_aux_c,(!(i%10)?"\n":""));
    }
    freeze_va_no_ts("\n");
    // age of measured values
    freeze_va("[age: ubatt %u temp %u]", get_age_ticks_u_batt(), get_age_ticks_temp());
    // calculated values
    freeze_va_no_ts("[%u %u %u mV] [%d %d %d battC] [%d %d %d auxC] [%d %d %d caseBotC] [%d shuntC]\n",
        module_data_stat.u_batt_mv_lowest, module_data_stat.u_batt_mv_mean, module_data_stat.u_batt_mv_highest,
        module_data_stat.temp_batt_c_lowest, module_data_stat.temp_batt_c_mean, module_data_stat.temp_batt_c_highest,
        module_data_stat.temp_aux_c_lowest, module_data_stat.temp_aux_c_mean, module_data_stat.temp_aux_c_highest,
        module_data_stat.temp_case_bottom_c_lowest, module_data_stat.temp_case_bottom_c_mean, module_data_stat.temp_case_bottom_c_highest,
        module_data_stat.temp_shunt_c);
}
/* store all calculated cellmodule data values and ages */
void freezeframe_cellmodule_compact_debug()
{
    // calculated values
    freeze_va("[%u %u %u mV] [%d %d %d battC] [%d %d %d auxC] [%d %d %d caseBotC] [%d shuntC]\n",
        module_data_stat.u_batt_mv_lowest, module_data_stat.u_batt_mv_mean, module_data_stat.u_batt_mv_highest,
        module_data_stat.temp_batt_c_lowest, module_data_stat.temp_batt_c_mean, module_data_stat.temp_batt_c_highest,
        module_data_stat.temp_aux_c_lowest, module_data_stat.temp_aux_c_mean, module_data_stat.temp_aux_c_highest,
        module_data_stat.temp_case_bottom_c_lowest, module_data_stat.temp_case_bottom_c_mean, module_data_stat.temp_case_bottom_c_highest,
        module_data_stat.temp_shunt_c);
}
