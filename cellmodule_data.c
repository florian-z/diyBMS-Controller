#include "cellmodule_data.h"

#include "log_util.h"
#include "uart_cellmodule.h"

// measurement of all cell modules
module_data_t module_data[MODULE_DATA_LEN] = {0};
// age of measurement data of both cellmodule-chains in ticks
module_data_age_t module_data_age[CELLMODULE_CHANNELS] = {0};
// min, max, mean, etc. values of module_data
module_data_statistics_t module_data_stat = {0};


#define BOTTOM_CASE_AUX_TEMP_ID_LEN 4
static const uint8_t BOTTOM_CASE_AUX_TEMP_IDS[BOTTOM_CASE_AUX_TEMP_ID_LEN] = { 1, 9, 37, 45 };
#define SHUNT_AUX_TEMP 2



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
                tmp_low = module_data[i].temp_aux_c;
            }
            if (module_data[BOTTOM_CASE_AUX_TEMP_IDS[i]].temp_aux_c > tmp_high) {
                tmp_high = module_data[i].temp_aux_c;
            }
            tmp_sum += module_data[BOTTOM_CASE_AUX_TEMP_IDS[i]].temp_aux_c;
        }
        module_data_stat.temp_case_bottom_c_lowest = tmp_low;
        module_data_stat.temp_case_bottom_c_highest = tmp_high;
        module_data_stat.temp_case_bottom_c_mean = (uint8_t)(tmp_sum / (float)CELLMODULES_TOTAL);
    }
    {
        module_data_stat.temp_shunt_c = module_data[SHUNT_AUX_TEMP].temp_aux_c;
    }
}

// use heater until all cells above LOW and none above HIGH
#define LIMITS_TEMP_HEATER_NEEDED_LOW 20
#define LIMITS_TEMP_HEATER_SAFETY_HIGH 30
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
        module_data_age[chain_id].u_batt++;
        module_data_age[chain_id].temp++;
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

/* print all cellmodule data values and ages */
void print_cellmodule_full_debug()
{
    // measured values
    for(uint8_t i=0; i<=CELLMODULES_TOTAL; i++)
    {
        log_va("[%d: %dmV: %dC: %dC] ", i, module_data[i].u_batt_mv, module_data[i].temp_batt_c, module_data[i].temp_aux_c);
    }
    log_va("\n");
    // age of measured values
//    for(uint8_t i=0; i<CELLMODULE_CHANNELS; i++)
//    {
//        log_va("debug cell age: chain %d ubatt %u temp %u\n", i, module_data_age[i].u_batt, module_data_age[i].temp);
//    }
    log_va("[age: ubatt %u temp %u]", get_age_ticks_u_batt(), get_age_ticks_temp());
    // calculated values
    log_va("[%u %u %u mV] [%d %d %d battC] [%d %d %d auxC] [%d %d %d caseBotC] [%d shuntC]\n",
        module_data_stat.u_batt_mv_lowest, module_data_stat.u_batt_mv_mean, module_data_stat.u_batt_mv_highest,
        module_data_stat.temp_batt_c_lowest, module_data_stat.temp_batt_c_mean, module_data_stat.temp_batt_c_highest,
        module_data_stat.temp_aux_c_lowest, module_data_stat.temp_aux_c_mean, module_data_stat.temp_aux_c_highest,
        module_data_stat.temp_case_bottom_c_lowest, module_data_stat.temp_case_bottom_c_mean, module_data_stat.temp_case_bottom_c_highest,
        module_data_stat.temp_shunt_c);
}
