#include "bluetooth.h"

#include "log_util.h"
#include "uart_ble.h"
#include "time_util.h"

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
    // TODO send binary
    send_message_ble_binary((uint8_t*)&msg, msg[BLE_LEN_L]+4);
}

void recv_ble_msg(uint8_t* msg, uint8_t msglen)
{
    if(msg[BLE_OPCODE] == SetTimestamp_0x86)
    {
        log("BLE Set Timestamp: ");
        if(msg[BLE_LEN_H] == 0 && msg[BLE_LEN_L] == 5 && msglen == 9)
        {
            uint32_t ts = msg[BLE_OPCODE+1]<<24 | msg[BLE_OPCODE+2]<<16 | msg[BLE_OPCODE+3]<<8 | msg[BLE_OPCODE+4];
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
