#include "uart_usb.h"
#include <string.h>
#include <stdlib.h>

#include "messages.h"
#include "uart_cellmodule.h"
#include "cellmodule_data.h"
#include "shunt.h"
#include "log_util.h"
#include "freeze_util.h"
#include "storage_util.h"

/*** USB / Debug UART ***/
static volatile uint8_t send_buf_usb[TX_BUF_USB] = {0};     /* transmit buffer */
static volatile uint8_t* send_buf_usb_wr = send_buf_usb;    /* write pointer */
static volatile uint8_t* send_buf_usb_rd = send_buf_usb;    /* read pointer */
static volatile bool tx_usb_busy = false;                   /* transmit active */
void send_message_usb_trigger_send(void);

static volatile uint8_t process_buffer_usb[RX_BUF_USB] = {0};   /* recive buffer */

/* buffer incoming messages */
void send_message_usb(uint8_t const * const data)
{
    GLOBAL_INT_STORE_AND_DISABLE
    const uint8_t DATA_LEN = strlen((char*)data);
    for(uint8_t i = 0; i<DATA_LEN; i++)
    {
        /* ensure no data gets overwritten */
        if(*send_buf_usb_wr != '\0')
        {
            /* buffer full -> skip */
            *(send_buf_usb_wr-1) = '\0'; // ensure previous string has a defined end
            break;
        }
        *send_buf_usb_wr++ = data[i];
        /* write pointer roll over */
        if(send_buf_usb_wr >= send_buf_usb + TX_BUF_USB - 1)
        {
            send_buf_usb_wr = send_buf_usb;
        }
    }
    /* start transmission */
    if(!tx_usb_busy)
    {
        send_message_usb_trigger_send();
    }
    GLOBAL_INT_RESTORE
}

/* called when ready for next transmission, checks for further pending transmission */
void send_message_usb_done(void)
{
    GLOBAL_INT_STORE_AND_DISABLE
    if(*send_buf_usb_rd != '\0')
    {
        /* continue transfer, if more data ready */
        send_message_usb_trigger_send();
    }
    else
    {
        tx_usb_busy = false;
    }
    GLOBAL_INT_RESTORE
}

/* INTERNAL: start UART TX*/
void send_message_usb_trigger_send(void)
{
    tx_usb_busy = true;
    uint16_t send_len16 = strlen((char*)send_buf_usb_rd);
    if (send_len16 > UINT8_MAX)
    {
        send_len16 = UINT8_MAX;
    }
    const uint8_t SEND_LEN8 = send_len16;

    R_Config_SCI6_USB_Serial_Send((uint8_t*)send_buf_usb_rd, SEND_LEN8, SCI_TX_ZERO);
    send_buf_usb_rd += SEND_LEN8;
    /* read pointer roll over */
    if(send_buf_usb_rd >= send_buf_usb + TX_BUF_USB - 1)
    {
        send_buf_usb_rd = send_buf_usb;
    }
}

void send_message_usb_no_buffer(uint8_t const * const send_data, uint16_t const send_len)
{
    GLOBAL_INT_STORE_AND_DISABLE
    if (!tx_usb_busy)
    {
        tx_usb_busy = true;
        R_Config_SCI6_USB_Serial_Send((uint8_t*)send_data, send_len, SCI_TX_NO_ZERO);
    }
    GLOBAL_INT_RESTORE
}

/* buffer incoming messages */
void pass_message_usb(uint8_t const * const data, uint8_t const len)
{
    if (process_buffer_usb[0] == '\0')
    {
        /* buffer is empty / not in use -> store message */
        strncpy((char*)process_buffer_usb, (char*)data, len);
    }
    else
    {
        Error_Handler();
    }
}

/* process buffered incoming messages */
void process_message_usb()
{
    if (process_buffer_usb[0] != '\0')
    {
        /* buffer is not empty -> process message */
        if (!strncmp("CHAIN_CMD", (char*)process_buffer_usb, 9))
        {
            uint8_t* ptr = (uint8_t*)process_buffer_usb + 9; // advance behind "CHAIN_CMD"
            ptr[strlen((char*)ptr)-1] = '\0'; // remove '\n'

            append_nmea_crc(ptr);
            log_va(">CMD: %s", ptr);  // TODO flo: debug remove
            send_message_cellmodule(ptr); // send to all chains
        }
        else if (!strncmp("CELL_CMD", (char*)process_buffer_usb, 8))
        {
            uint8_t* ptr = (uint8_t*)process_buffer_usb + 8; // advance behind "CELL_CMD"
            uint8_t module_id = atoi((char*)ptr); // get target module-id
            ptr += 2; // move behind target module-id
            ptr[strlen((char*)ptr)-1] = '\0'; // remove '\n'
            // add module_count to message
            if (module_id > CELLMODULES_CHANNEL_1)
            {
                format_byte_to_chars(ptr+3, -(module_id -1 -CELLMODULES_CHANNEL_1));
            }
            else
            {
                format_byte_to_chars(ptr+3, -(module_id -1));
            }

            append_nmea_crc(ptr);
            log_va(">CMD: %d %s", module_id, ptr);  // TODO flo: debug remove
            send_message_cellmodule_specific(ptr, module_id); // only send to chain containing module-id
        }
        else if (!strncmp("CELL_CFG", (char*)process_buffer_usb, 8))
        {
            uint8_t* ptr = (uint8_t*)process_buffer_usb + 8; // advance behind "CELL_CFG"
            uint8_t module_id = atoi((char*)ptr); // get target module-id
            ptr += 3; // move behind target module-id
            uint16_t cfg_value = atoi((char*)ptr); // get target cfg_value
            ptr += 4; // move behind target cfg_value
            ptr[strlen((char*)ptr)-1] = '\0'; // remove '\n'
            // add module_count to message
            if (module_id > CELLMODULES_CHANNEL_1)
            {
                format_byte_to_chars(ptr+3, -(module_id -1 -CELLMODULES_CHANNEL_1));
            }
            else
            {
                format_byte_to_chars(ptr+3, -(module_id -1));
            }
            format_word_to_chars(ptr+3+2, cfg_value);

            append_nmea_crc(ptr);
            log_va(">CMD: %d %s", module_id, ptr);  // TODO flo: debug remove
            send_message_cellmodule_specific(ptr, module_id); // only send to chain containing module-id
        }
        else if (!strncmp("HELP", (char*)process_buffer_usb, 4))
        {
            log("CHAIN_CMD!<cmd>00  !0000 or !0100\nCELL_CMD<module>!<cmd>00 cmd:00..03, 07..08  01!0002\nCELL_CFG<module> <value>!<cmd>00  01 255!0004\nHELP, BATT, SHUNT, FREEZE, TIME, DIRECT_BAL_ON/OFF, DIRECT_HEAT_ON/OFF, CL_STATUS, CL_STATUS_FREEZE\n");
            log("GET_BATT_VOLT=0, GET_TEMP=1, IDENTIFY_MODULE=2, ACTIVATE_POWERSAVE=3, SET_CONFIG_BATT_VOLT_CALIB=4, SET_CONFIG_TEMP1_B_COEFF=5, SET_CONFIG_TEMP2_B_COEFF=6, GET_CONFIG=7, CLEAR_CONFIG=8\n");
        }
        else if (!strncmp("BATT", (char*)process_buffer_usb, 4))
        {
            log_cellmodule_full_debug();
        }
        else if (!strncmp("SHUNT", (char*)process_buffer_usb, 5))
        {
            log_shunt_full_debug();
        }
        else if (!strncmp("FREEZE", (char*)process_buffer_usb, 6))
        {
//            if (give_buffer_start() != give_history_start())
//            {
//                send_message_usb_no_buffer(give_history_start(), strlen((char*)give_history_start()));
//            }
//            send_message_usb_no_buffer(give_buffer_start(), strlen((char*)give_buffer_start()));
            send_message_usb_no_buffer(give_buffer_start(), STORAGE_UTIL_LEN);
            log("END\n");
        }
        else if (!strncmp("TIME", (char*)process_buffer_usb, 4))
        {
            log_va("[TIME %s]\n", get_ts_full_str());
        }
        else if (!strncmp("DIRECT_BAL_ON", (char*)process_buffer_usb, 13))
        {
            freeze("[UART-USB: DIRECT BAL ON]\n");
            cl_balancer_on();
        }
        else if (!strncmp("DIRECT_BAL_OFF", (char*)process_buffer_usb, 14))
        {
            freeze("[UART-USB: DIRECT BAL OFF]\n");
            cl_balancer_off();
        }
        else if (!strncmp("DIRECT_HEAT_ON", (char*)process_buffer_usb, 14))
        {
            freeze("[UART-USB: DIRECT HEAT ON]\n");
            cl_heater_on();
        }
        else if (!strncmp("DIRECT_HEAT_OFF", (char*)process_buffer_usb, 15))
        {
            freeze("[UART-USB: DIRECT HEAT OFF]\n");
            cl_heater_off();
        }
        else if (!strncmp("CL_STATUS", (char*)process_buffer_usb, 9))
        {
            print_charger_logic_status();
        }
        else if (!strncmp("CL_STATUS_FREEZE", (char*)process_buffer_usb, 16))
        {
            freeze_charger_logic_status();
        }
        else
        {
            log_va("CMD UNKNOWN: \"%s\"\n", process_buffer_usb);
        }

        /* last step: free buffer */
        memset((uint8_t*)process_buffer_usb, '\0', RX_BUF_USB);
    }
}
