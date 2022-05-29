#ifndef __cellmodule_h__
#define __cellmodule_h__

#include "main.h"
#include "r_cg_userdefine.h"

#define MSG_CMD 1
#define MSG_MOD_CNT 3
#define MSG_DATA_BEGIN 5


typedef enum msg_command { GET_BATT_VOLT=0, GET_TEMP=1, IDENTIFY_MODULE=2, ACTIVATE_POWERSAVE=3,
   SET_CONFIG_BATT_VOLT_CALIB=4, SET_CONFIG_TEMP1_B_COEFF=5, SET_CONFIG_TEMP2_B_COEFF=6, GET_CONFIG=7, CLEAR_CONFIG=8  } msg_command_t;

typedef struct {
    float u_batt_mv;
    uint8_t temp_batt_c;
    uint8_t temp_aux_c;
} module_data_t;

typedef struct  {
    uint16_t u_batt;
    uint16_t temp;
} module_data_age_t;

void send_message_cellmodule(uint8_t const * const data);
void send_message_cellmodule_done(uint8_t const chain_no);
void pass_message_cellmodule(uint8_t const * const data, uint8_t const len, uint8_t const chain_no);
void process_message_cellmodule();

void tick_cellmodule();
uint16_t get_age_ticks_u_batt();
uint16_t get_age_ticks_temp();

#endif /* __cellmodule_h__ */