#include "bluetooth.h"

#include <string.h>

#include "log_util.h"
#include "uart_ble.h"
#include "time_util.h"
#include "cellmodule_data.h"
#include "uart_cellmodule.h"
#include "shunt.h"
#include "storage_util.h"

extern module_data_t module_data[MODULE_DATA_LEN];
extern module_data_age_t module_data_age[CELLMODULE_CHANNELS];
extern module_data_statistics_t module_data_stat;
extern shunt_t shunt_data;

void bluetooth_init_config_mode()
{
    OUT_BLE_RESET_RESET
    OUT_BLE_MODE_CONFIG
    R_BSP_SoftwareDelay(3, BSP_DELAY_MILLISECS); // 1ms required
    OUT_BLE_RESET_RUN
    // wait a little (25ms)
    R_BSP_SoftwareDelay(50, BSP_DELAY_MILLISECS); // 25ms required
    OUT_BLE_MODE_RUN
    // wait a little (21ms)
    R_BSP_SoftwareDelay(50, BSP_DELAY_MILLISECS); // 21ms required
}

void bluetooth_init_run_mode()
{
    OUT_BLE_RESET_RESET
    OUT_BLE_MODE_RUN
    R_BSP_SoftwareDelay(3, BSP_DELAY_MILLISECS); // 1ms required
    OUT_BLE_RESET_RUN
    // wait a little (68ms <- 25ms+43ms)
    R_BSP_SoftwareDelay(100, BSP_DELAY_MILLISECS); // 68ms required
}

void send_ble_cmd(ble_cmd_t ble_cmd)
{
    //ble_gmf_t msg = {0};
    uint8_t msg[TX_BUF_BLE] = {0};
    msg[BLE_SYNC_ID] = BLE_SYNC_WORD;

    switch(ble_cmd) {
        case Read_Local_Info_0x01:
            log("BLE: read local info\n");
            msg[BLE_OPCODE] = 0x01;
            msg[BLE_LEN_L] = 1;
            msg[BLE_OPCODE+1] = 254;
            break;
        case Reset_0x02:
            log("BLE: do reset\n");
            break;
        case Read_Status_0x03:
            log("BLE: read status\n");
            break;
        default:
            log_va("unknown ble cmd: %02X\n", ble_cmd);
            return;
    }


    // TODO calc crc
    //uint8_t msg2[] =
    send_message_ble_binary((uint8_t*)&msg, msg[BLE_LEN_L]+4);
}

void send_ble_android()
{
    static uint8_t msg[TX_BUF_BLE] = {0};
    memset(msg, 0, TX_BUF_BLE);
    static uint8_t msgid = 0;
    uint8_t used_len = 0;
    uint8_t* msg_start = "";
    uint8_t msg_len = 0;
    switch(++msgid) {
        case 1:
            msg[0] = msgid;
            msg[1] = MODULE_DATA_LEN;
            for(uint8_t i = 1; i<MODULE_DATA_LEN; i++)
            {
                memcpy(&msg[i*2], &module_data[i].u_batt_mv, 2);
            }
            msg_len = 96;
            break;
        case 2:
            msg[0] = msgid;
            msg[1] = MODULE_DATA_LEN;
            for(uint8_t i = 1; i<MODULE_DATA_LEN; i++)
            {
                memcpy(&msg[i], &module_data[i].temp_batt_c, 1);
            }
            msg_len = 49;
            break;
        case 3:
            msg[0] = msgid;
            msg[1] = MODULE_DATA_LEN;
            for(uint8_t i = 1; i<MODULE_DATA_LEN; i++)
            {
                memcpy(&msg[i], &module_data[i].temp_aux_c, 1);
            }
            msg_len = 49;
            break;
        case 4:
            msg[0] = msgid;
            msg[1] = 7;
            memcpy(&msg[2], &shunt_data.vshunt, 4);
            memcpy(&msg[6], &shunt_data.vbus, 4);
            memcpy(&msg[10], &shunt_data.dietemp, 4);
            memcpy(&msg[14], &shunt_data.current, 4);
            memcpy(&msg[18], &shunt_data.power, 4);
            memcpy(&msg[22], &shunt_data.energy, 4);
            memcpy(&msg[26], &shunt_data.charge, 4);

            memcpy(&msg[30], &module_data[BOTTOM_CASE_AUX_TEMP_ID_1].temp_aux_c, 1);
            memcpy(&msg[31], &module_data[BOTTOM_CASE_AUX_TEMP_ID_2].temp_aux_c, 1);
            memcpy(&msg[32], &module_data[BOTTOM_CASE_AUX_TEMP_ID_3].temp_aux_c, 1);
            memcpy(&msg[33], &module_data[BOTTOM_CASE_AUX_TEMP_ID_4].temp_aux_c, 1);
            memcpy(&msg[34], &SHUNT_AUX_TEMP, 1);

            memcpy(&msg[35], &module_data_age[CELL_MODULE_CHAIN_1].u_batt, 2);
            memcpy(&msg[37], &module_data_age[CELL_MODULE_CHAIN_2].u_batt, 2);
            memcpy(&msg[39], &module_data_age[CELL_MODULE_CHAIN_1].temp, 2);
            memcpy(&msg[41], &module_data_age[CELL_MODULE_CHAIN_2].temp, 2);

            memcpy(&msg[43], &module_data_stat.u_batt_mv_lowest, 2);
            memcpy(&msg[45], &module_data_stat.u_batt_mv_mean, 2);
            memcpy(&msg[47], &module_data_stat.u_batt_mv_highest, 2);
            memcpy(&msg[49], &module_data_stat.temp_batt_c_lowest, 1);
            memcpy(&msg[50], &module_data_stat.temp_batt_c_mean, 1);
            memcpy(&msg[51], &module_data_stat.temp_batt_c_highest, 1);
            memcpy(&msg[52], &module_data_stat.temp_aux_c_lowest, 1);
            memcpy(&msg[53], &module_data_stat.temp_aux_c_mean, 1);
            memcpy(&msg[54], &module_data_stat.temp_aux_c_highest, 1);
            memcpy(&msg[55], &module_data_stat.temp_case_bottom_c_lowest, 1);
            memcpy(&msg[56], &module_data_stat.temp_case_bottom_c_mean, 1);
            memcpy(&msg[57], &module_data_stat.temp_case_bottom_c_highest, 1);

            msg_len = 58;

            if (!has_ble_message())
            {
                // no message to send, continue with first case
                msgid = 0;
            }
            break;
        case 5:
            msg[0] = msgid;
            msg[1] = 7;
            used_len = 0;
            msg_start = give_ble_start_and_increment(TX_BUF_BLE-2-1, &used_len); // TX_BUF_BLE - two start bytes - trailing '\0'
            memcpy(&msg[2], msg_start, used_len);
            msg_len = used_len+2; // do not transmit trailing '\0'

            msgid = 0;
            break;
        default:
            msgid = 0;
            break;
    }

    send_message_ble_binary((uint8_t*)&msg, msg_len);
}

void recv_ble_msg(uint8_t* msg, uint8_t msglen)
{
    if(msg[BLE_OPCODE] == SetTimestamp_0x86)
    {
        log("BLE Set Timestamp: ");
        if(msg[BLE_LEN_H] == 0 && msg[BLE_LEN_L] == 5 && msglen == 9)
        {
            uint32_t ts;
            memcpy(&ts, &msg[BLE_OPCODE+1], 4);
            log_va("ts: %lu\n", ts);
            set_time_tick(ts);
        }
        else
        {
            log("ble cmd malformed\n");
        }
    }
    else
    {
        log_va("BLE unknown opcode: %02X\n", msg[BLE_OPCODE]);
    }
}
