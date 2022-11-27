#ifndef __bluetooth_h__
#define __bluetooth_h__

#include "main.h"
#include "r_cg_userdefine.h"

#define advertisement_UUID "B940FEC7FAB642DCA2E271A5F4C74198"


typedef enum
{
    Read_Local_Info_0x01 = 1,
    Reset_0x02 = 2,
    Read_Status_0x03 = 3,

    SetTimestamp_0x86 = 0x86
} ble_cmd_t;

#define BLE_SYNC_WORD   0xAA
//typedef struct
//{
//    uint8_t start;          // static sync word
//    uint16_t len;           // only op_code + params
//    uint8_t op_code;
//    uint8_t* params;
//    uint8_t crc;            // sum of bytes (len + op_code + params) -> &0xff -> -1 -> bit inverse
//} ble_gmf_t;

#define BLE_SYNC_ID     0
#define BLE_LEN_H       1
#define BLE_LEN_L       2
#define BLE_OPCODE      3


void bluetooth_init_config_mode();
void bluetooth_init_run_mode();
void send_ble_cmd(ble_cmd_t ble_cmd);
void send_ble_android();
void recv_ble_msg(uint8_t* msg, uint8_t msglen);

#endif /* __bluetooth_h__ */