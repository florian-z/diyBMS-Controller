#ifndef __uart_cellmodule_h__
#define __uart_cellmodule_h__

#include "main.h"
#include "r_cg_userdefine.h"

#define MSG_CMD 1
#define MSG_MOD_CNT 3
#define MSG_DATA_BEGIN 5

#define CELLMODULE_CHANNELS 2
#define CELLMODULES_CHANNEL_1 24
#define CELLMODULES_CHANNEL_2 23
#define CELLMODULES_TOTAL (CELLMODULES_CHANNEL_1+CELLMODULES_CHANNEL_2)
#define CELLMODULES_FIRST 1
#define MODULE_DATA_LEN (CELLMODULES_TOTAL+CELLMODULES_FIRST)

typedef enum msg_command { GET_BATT_VOLT=0, GET_TEMP=1, IDENTIFY_MODULE=2, ACTIVATE_POWERSAVE=3,
   SET_CONFIG_BATT_VOLT_CALIB=4, SET_CONFIG_TEMP1_B_COEFF=5, SET_CONFIG_TEMP2_B_COEFF=6, GET_CONFIG=7, CLEAR_CONFIG=8  } msg_command_t;



void send_message_cellmodule(uint8_t const * const data);
void send_message_cellmodule_specific(uint8_t const * const data, uint8_t const module_id);
void send_message_cellmodule_done(uint8_t const chain_no);
void pass_message_cellmodule(uint8_t const * const data, uint8_t const len, uint8_t const chain_no);
void process_message_cellmodule();

#endif /* __uart_cellmodule_h__ */