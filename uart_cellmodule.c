#include "uart_cellmodule.h"
#include <string.h>
#include "log_util.h"
#include "messages.h"
#include "cellmodule_data.h"

//#define CELLMODULE_CHANNELS 2
//static const uint8_t CELLMODULES_PER_CHAIN[CELLMODULE_CHANNELS] = { 24, 23 };
//static const uint8_t START_OF_CHAIN[CELLMODULE_CHANNELS] = { 1, 25 };
//#define CELLMODULES_TOTAL 47

static const uint8_t CELLMODULES_PER_CHAIN[CELLMODULE_CHANNELS] = { CELLMODULES_CHANNEL_1, CELLMODULES_CHANNEL_2 };
static const uint8_t START_OF_CHAIN[CELLMODULE_CHANNELS] = { CELLMODULES_FIRST, CELLMODULES_FIRST+CELLMODULES_CHANNEL_1 };


/*** CellModules UART ***/
static volatile uint8_t cellmodule_process_buf[CELLMODULE_CHANNELS][RX_BUF_CELLMODULE] = {0};
static volatile uint8_t cellmodule_tx_buf[CELLMODULE_CHANNELS][TX_BUF_CELLMODULE] = {0};
static volatile uint8_t cellmodule_tx_busy = 0;
static volatile uint8_t cellmodule_rx_waiting_for_response = 0;

extern module_data_t module_data[MODULE_DATA_LEN];
extern module_data_age_t module_data_age[CELLMODULE_CHANNELS];

/* generate and send messages */
/* send message to all channels */
void send_message_cellmodule(uint8_t const * const data)
{
    LED_GN1_ON
    LED_GN2_ON
    GLOBAL_INT_STORE_AND_DISABLE
    // check that no channel is currently transmitting
    if (cellmodule_tx_busy)
    {
        log_va("cell channel %02X is transmitting\n", cellmodule_tx_busy);
    }
    else
    {
//        log_va("out all cell: %s", data);
        // mark transmit of all chains as busy
        cellmodule_tx_busy = (1<<CELL_MODULE_CHAIN_1) | (1<<CELL_MODULE_CHAIN_2);
        // mark waiting-for-response of all chains as waiting
        cellmodule_rx_waiting_for_response = (1<<CELL_MODULE_CHAIN_1) | (1<<CELL_MODULE_CHAIN_2);

        const uint8_t DATA_LEN = strlen((char*)data);
        memcpy((uint8_t*)cellmodule_tx_buf[CELL_MODULE_CHAIN_1], data, DATA_LEN);
        R_Config_SCI0_CellModule_Serial_Send((uint8_t*)cellmodule_tx_buf[CELL_MODULE_CHAIN_1], DATA_LEN);

        memcpy((uint8_t*)cellmodule_tx_buf[CELL_MODULE_CHAIN_2], data, DATA_LEN);
        R_Config_SCI5_CellModule_Serial_Send((uint8_t*)cellmodule_tx_buf[CELL_MODULE_CHAIN_2], DATA_LEN);
    }
    GLOBAL_INT_RESTORE
}

/* send message to specific channel */
void send_message_cellmodule_specific(uint8_t const * const data, uint8_t const module_id)
{
    uint8_t chain_id = CELL_MODULE_CHAIN_1;
    LED_GN1_ON
    if (module_id > CELLMODULES_CHANNEL_1)
    {
        LED_GN2_ON
        LED_GN1_OFF
        chain_id = CELL_MODULE_CHAIN_2;
    }
    GLOBAL_INT_STORE_AND_DISABLE
    // check that no channel is currently transmitting
    if (cellmodule_tx_busy)
    {
        log_va("cell channel %02X is transmitting\n", cellmodule_tx_busy);
    }
    else
    {
        log_va("out single cell: %s", data);
        // mark transmit of all chains as busy
        cellmodule_tx_busy = (1<<chain_id);
        // mark waiting-for-response of all chains as waiting
        cellmodule_rx_waiting_for_response = (1<<chain_id);

        const uint8_t DATA_LEN = strlen((char*)data);
        if (chain_id == CELL_MODULE_CHAIN_1)
        {
            memcpy((uint8_t*)cellmodule_tx_buf[CELL_MODULE_CHAIN_1], data, DATA_LEN);
            R_Config_SCI0_CellModule_Serial_Send((uint8_t*)cellmodule_tx_buf[CELL_MODULE_CHAIN_1], DATA_LEN);
        }
        else
        {
            memcpy((uint8_t*)cellmodule_tx_buf[CELL_MODULE_CHAIN_2], data, DATA_LEN);
            R_Config_SCI5_CellModule_Serial_Send((uint8_t*)cellmodule_tx_buf[CELL_MODULE_CHAIN_2], DATA_LEN);
        }
    }
    GLOBAL_INT_RESTORE
}

/* called when transmission done, could check for further pending transmission */
void send_message_cellmodule_done(uint8_t const chain_no)
{
    GLOBAL_INT_STORE_AND_DISABLE
    // mark transmit of this chain as free (not busy transmitting)
    cellmodule_tx_busy &= ~(1<<chain_no);
    GLOBAL_INT_RESTORE
}

/* buffer incoming messages */
void pass_message_cellmodule(uint8_t const * const data, uint8_t const len, uint8_t const chain_no)
{
    GLOBAL_INT_STORE_AND_DISABLE
    // mark waiting-for-response of this chain as free (not waiting for response)
    cellmodule_rx_waiting_for_response &= ~(1<<chain_no);
//    if (CELL_MODULE_CHAIN_1 == chain_no) {
//        LED_GN1_OFF
//    } else if (CELL_MODULE_CHAIN_2 == chain_no) {
//        LED_GN2_OFF
//    }

    if (cellmodule_process_buf[chain_no][0] == '\0')
    {
        // buffer is empty / not in use -> store message
        memcpy((uint8_t*)cellmodule_process_buf[chain_no], data, len);
    }
    else
    {
        // buffer is not free -> error
        Error_Handler();
    }
    GLOBAL_INT_RESTORE
}

/* process buffered incoming messages */
void process_message_cellmodule_int(uint8_t const chain_no);
void process_message_cellmodule()
{
    for(uint8_t chain_no = 0; chain_no < CELLMODULE_CHANNELS; chain_no++)
    {
        if (cellmodule_process_buf[chain_no][0] != '\0')
        {
            /* buffer is not empty -> process message */
            //log_va("cell %d: %s", chain_no, (uint8_t*)cellmodule_process_buf[chain_no]);  // TODO flo: debug remove
            /* check message crc */
            if (!is_nmea_checksum_good((uint8_t*)cellmodule_process_buf[chain_no]))
            {
                //  checksum bad
                log_va("badcrc cell %d: %s", chain_no, (uint8_t*)cellmodule_process_buf[chain_no]);  // TODO flo: debug remove
            }
            else
            {
                // checksum ok
                // clear MSG_CRC, <crc> and MSG_END
                {
                    uint8_t* ptr = (uint8_t*)cellmodule_process_buf[chain_no];
                    while(MSG_CRC != *++ptr);
                    *ptr++ = '\0'; // '*'
                    *ptr++ = '\0'; // <crc1>
                    *ptr++ = '\0'; // <crc2>
                    *ptr = '\0';   //'\n'
                }

                if (CELL_MODULE_CHAIN_1 == chain_no)
                {
                    LED_GN1_OFF
                    report_system_status(CELLMODULE_CHAIN1);
                }
                else if (CELL_MODULE_CHAIN_2 == chain_no)
                {
                    LED_GN2_OFF
                    report_system_status(CELLMODULE_CHAIN2);
                }
                process_message_cellmodule_int(chain_no);
            }
            /* last step: free buffer */
            memset((uint8_t*)cellmodule_process_buf[chain_no], '\0', RX_BUF_CELLMODULE);
        }
        else
        {
            /* buffer is empty -> nothing to do here */
        }
    }
}

void process_message_cellmodule_int(uint8_t const chain_no)
{
    uint8_t const * const msg_ptr = (uint8_t*)cellmodule_process_buf[chain_no];
    uint8_t const msg_cmd = parse_chars_to_byte(msg_ptr + MSG_CMD);
    // local module id (within this chain)
    uint8_t const module_cnt = parse_chars_to_byte(msg_ptr + MSG_MOD_CNT);
    // global module id
    uint8_t const module_id = CELLMODULES_PER_CHAIN[chain_no] - module_cnt + START_OF_CHAIN[chain_no];
    uint8_t const * msg_data_ptr = msg_ptr + MSG_DATA_BEGIN;

    if (CELLMODULES_PER_CHAIN[chain_no] != module_cnt)
    {
        // incorrect number of modules in response
        log_va("modulecnt cell %d: %s [cmd:%d]\n", chain_no, msg_ptr, msg_cmd);
        return;
    }
    //log_va("log cell %d: %s [cmd:%d mcnt:%d]\n", chain_no, msg_ptr, msg_cmd, module_cnt);

    switch(msg_cmd)
    {
        case GET_BATT_VOLT:
            // read volts to structs
            for(uint8_t mod_id = START_OF_CHAIN[chain_no]; mod_id < START_OF_CHAIN[chain_no] + CELLMODULES_PER_CHAIN[chain_no]; mod_id++)
            {
                module_data[mod_id].u_batt_mv = parse_chars_to_word(msg_data_ptr);
                msg_data_ptr += 4; // advance by one word
            }
            //log_va("log cell %d: UBATT %s\n", chain_no, msg_ptr);
            module_data_age[chain_no].u_batt = 0;
            break;

        case GET_TEMP:
            // read temps to structs
            for(uint8_t mod_id = START_OF_CHAIN[chain_no]; mod_id < START_OF_CHAIN[chain_no] + CELLMODULES_PER_CHAIN[chain_no]; mod_id++)
            {
                module_data[mod_id].temp_batt_c = parse_chars_to_byte(msg_data_ptr);
                msg_data_ptr += 2; // advance by one byte
                module_data[mod_id].temp_aux_c = parse_chars_to_byte(msg_data_ptr);
                msg_data_ptr += 2; // advance by one byte
            }
            //log_va("log cell %d: TEMP %s\n", chain_no, msg_ptr);
            module_data_age[chain_no].temp = 0;
            break;

        case IDENTIFY_MODULE:
            // ignore, just debug
            log_va("log cell %d: IDENTIFY[%d] %s\n", chain_no, module_id, msg_ptr);
            break;

        case ACTIVATE_POWERSAVE:
            // ignore, just debug
            log_va("log cell %d: PWRSAFE %s\n", chain_no, msg_ptr);
            break;

        case SET_CONFIG_BATT_VOLT_CALIB:
        {
            // ignore, just debug
            uint16_t ubatt_mv_default_calib = parse_chars_to_word(msg_ptr+5+4);
            uint16_t ubatt_mv_after = parse_chars_to_word(msg_ptr+5+4+4);
            uint16_t volt_calib_value_after = parse_chars_to_word(msg_ptr+5+4+4+4);
            log_va("log cell %d: SET_CFG_UBATT[%d] %s default-calib-ubatt:%d new-ubatt:%d new-calib:%d\n", chain_no, module_id, msg_ptr, ubatt_mv_default_calib, ubatt_mv_after, volt_calib_value_after);
            break;
        }
        case SET_CONFIG_TEMP1_B_COEFF:
        {
            // ignore, just debug
            uint16_t temp_b_coeff = parse_chars_to_word(msg_ptr+5+4);
            int8_t temp_before = parse_chars_to_byte(msg_ptr+5+4+4);
            int8_t temp_after = parse_chars_to_byte(msg_ptr+5+4+4+2);
            log_va("log cell %d: SET_CFG_TEMP1[%d] %s new-b-coeff:%d before:%d after:%d\n", chain_no, module_id, msg_ptr, temp_b_coeff, temp_before, temp_after);
            break;
        }
        case SET_CONFIG_TEMP2_B_COEFF:
        {
            // ignore, just debug
            uint16_t temp_b_coeff = parse_chars_to_word(msg_ptr+5+4);
            int8_t temp_before = parse_chars_to_byte(msg_ptr+5+4+4);
            int8_t temp_after = parse_chars_to_byte(msg_ptr+5+4+4+2);
            log_va("log cell %d: SET_CFG_TEMP2[%d] %s new-b-coeff:%d before:%d after:%d\n", chain_no, module_id, msg_ptr, temp_b_coeff, temp_before, temp_after);
            break;
        }
        case CLEAR_CONFIG:
        {
            // ignore, just debug
            log_va("log cell %d: CLR_CFG[%d] %s\n", chain_no, module_id, msg_ptr);
            // no break here-> this should give the same output as GET_CONFIG
        }
        case GET_CONFIG:
        {
            // ignore, just debug
            log_va("log cell %d: GET_CFG[%d] %s\n", chain_no, module_id, msg_ptr);
            float adc_scale = parse_chars_to_dword(msg_ptr+5) / 1e11;
            uint16_t temp_int_b_coeff = parse_chars_to_word(msg_ptr+5+8);
            uint16_t temp_aux_b_coeff = parse_chars_to_word(msg_ptr+5+8+4);
            log_va("%f %d %d\n", adc_scale, temp_int_b_coeff, temp_aux_b_coeff);
            break;
        }
        default:
            // ignore, just debug
            log_va("log cell %d: UNKNOWN %s\n", chain_no, msg_ptr);
            break;
    }
}

