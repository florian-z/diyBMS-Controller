#include "uart_ble.h"
#include <string.h>
#include <stdlib.h>

#include "log_util.h"


/*** Display / BT UART ***/
static volatile uint8_t tx_ble_buf[TX_BUF_BLE] = {0};   /* transmit buffer */
static volatile uint8_t tx_ble_buf_len = 0;             /* transmit length */
static volatile bool tx_ble_busy = false;               /* transmit active */

void send_message_ble_trigger_send(void);

static volatile uint8_t process_buffer_ble[RX_BUF_BLE] = {0};   /* receive buffer*/
static volatile uint8_t process_buffer_ble_len = 0;

/* generate and send messages */
void send_message_ble_binary(uint8_t const * const data, uint8_t const data_len)
{
    GLOBAL_INT_STORE_AND_DISABLE
    if(tx_ble_busy)
    {
        // ble uart currently busy
        log("BLE:tx busy\n");
        GLOBAL_INT_RESTORE
        return;
    }
    tx_ble_busy = true;
    GLOBAL_INT_RESTORE

    memset((uint8_t*)tx_ble_buf, '\0', TX_BUF_BLE);
    memcpy((uint8_t*)tx_ble_buf, data, data_len);
    tx_ble_buf_len = data_len;

    R_Config_SCI1_BLE_Serial_Send((uint8_t*)tx_ble_buf, tx_ble_buf_len);
}

/* called when ready for next transmission */
void send_message_ble_done(void)
{
    tx_ble_busy = false;
}

/* buffer incoming messages */
void pass_message_ble(uint8_t const * const data, uint8_t const len)
{
    log("BLE:rx data\n");
    if (process_buffer_ble[0] == '\0')
    {
        /* buffer is empty / not in use -> store message */
        memcpy((uint8_t*)process_buffer_ble, data, len);
        process_buffer_ble_len = len;
    }
    else
    {
        Error_Handler();
    }
}

/* process buffered incoming messages */
void process_message_ble()
{
    if (process_buffer_ble[0] != '\0')
    {
        /* buffer is not empty -> process message */
        const uint8_t len = process_buffer_ble_len;
        log_hex((uint8_t*)process_buffer_ble, len);  // TODO flo: debug remove
        uint8_t sum = 0;
        for(uint8_t i = 3; i < len-1; i++)
        {
            sum += process_buffer_ble[i];
        }
        uint8_t crc = 255 - (sum - 1);
        if (process_buffer_ble[len-1] == crc)
        {
            // crc good
            log("ble crc good\n");
        }
        else
        {
            // crc bad
            log("ble crc bad\n");
        }
        /* last step: free buffer */
        memset((uint8_t*)process_buffer_ble, '\0', RX_BUF_BLE);
        process_buffer_ble_len = 0;
    }
}