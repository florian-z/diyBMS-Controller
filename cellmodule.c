#include "cellmodule.h"
#include "string.h"
#include "log_util.h"

#define CELLMODULE_CHANNELS 2

/*** CellModules UART ***/
static volatile uint8_t cellmodule_process_buf[CELLMODULE_CHANNELS][RX_BUF_CELLMODULE] = {0};
static volatile uint8_t cellmodule_tx_buf[CELLMODULE_CHANNELS][TX_BUF_CELLMODULE] = {0};
static volatile uint8_t cellmodule_tx_busy = 0;
static volatile uint8_t cellmodule_rx_waiting_for_response = 0;

/* generate and send messages */
void send_message_cellmodule(uint8_t const * const data)
{
    LED_GN1_ON
    GLOBAL_INT_STORE_AND_DISABLE
    // check that no channel is currently transmitting
    if (cellmodule_tx_busy)
    {
        log_va("cell channel %02X is transmitting\n", cellmodule_tx_busy);
    }
    else if (cellmodule_rx_waiting_for_response)
    {
        log_va("cell channel %02X waiting for response\n", cellmodule_rx_waiting_for_response);
    }
    else
    {
        log_va("\ncell: %s", data);
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
    if(!cellmodule_rx_waiting_for_response)
    {
        LED_GN1_OFF
    }
    GLOBAL_INT_RESTORE
}

/* process buffered incoming messages */
void process_message_cellmodule()
{
    for(uint8_t i = 0; i < CELLMODULE_CHANNELS; i++)
    {
        if (cellmodule_process_buf[i][0] != '\0')
        {
            /* buffer is not empty -> process message */
            log_va("cell %d: %s", i, (uint8_t*)cellmodule_process_buf[i]);  // TODO flo: debug remove
            /* last step: free buffer */
            memset((uint8_t*)cellmodule_process_buf[i], '\0', RX_BUF_CELLMODULE);
        }
    }
}
