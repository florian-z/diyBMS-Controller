#include "bluetooth.h"

#include "log_util.h"
#include "uart_ble.h"

void bluetooth_init_config_mode()
{
    OUT_BLE_RESET_RESET
    OUT_BLE_MODE_CONFIG
    R_BSP_SoftwareDelay(3, BSP_DELAY_MILLISECS); // 1ms required
    OUT_BLE_RESET_RUN
    // wait a little (25ms)
    R_BSP_SoftwareDelay(50, BSP_DELAY_MILLISECS); // 1ms required
    OUT_BLE_MODE_RUN
    // wait a little (21ms)
}

void bluetooth_init_run_mode()
{
    OUT_BLE_RESET_RESET
    OUT_BLE_MODE_RUN
    R_BSP_SoftwareDelay(3, BSP_DELAY_MILLISECS); // 1ms required
    OUT_BLE_RESET_RUN
    // wait a little (68ms <- 25ms+43ms)
}

void send_ble_cmd(ble_cmd_t ble_cmd)
{
    ble_gmf_t msg = {0};
    msg.start = BLE_SYNC_WORD;

    switch(ble_cmd) {
        case Read_Local_Info_0x01:
            log("BLE: read local info\n");
            msg.op_code = 0x01;
            msg.len = 1;
            msg.crc = 254;
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
    send_message_ble_binary((uint8_t*)&msg, msg.len+4);
}



