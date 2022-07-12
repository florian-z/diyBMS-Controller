#include "uart_ble.h"
#include <string.h>
#include <stdlib.h>

#include "log_util.h"


/*** Display / BT UART ***/
static volatile uint8_t send_buf_ble[TX_BUF_BLE] = {0};     /* transmit buffer */
static volatile uint8_t* send_buf_ble_wr = send_buf_ble;    /* write pointer */
static volatile uint8_t* send_buf_ble_rd = send_buf_ble;    /* read pointer */
static volatile bool tx_ble_busy = false;                   /* transmit active */
void send_message_ble_trigger_send(void);

static volatile uint8_t process_buffer_ble[RX_BUF_BLE] = {0};   /* receive buffer*/

/* generate and send messages */
void send_message_ble(uint8_t const * const data)
{
    GLOBAL_INT_STORE_AND_DISABLE
    const uint8_t DATA_LEN = strlen((char*)data);
    for(uint8_t i = 0; i<DATA_LEN; i++)
    {
        /* ensure no data gets overwritten */
        if(*send_buf_ble_wr != '\0')
        {
            /* buffer full -> skip */
            break;
        }
        *send_buf_ble_wr++ = data[i];
        /* write pointer roll over */
        if(send_buf_ble_wr >= send_buf_ble + TX_BUF_BLE - 1)
        {
            send_buf_ble_wr = send_buf_ble;
        }
    }
    /* start transmission */
    if(!tx_ble_busy)
    {
        send_message_ble_trigger_send();
    }
    GLOBAL_INT_RESTORE
}

/* called when ready for next transmission, checks for further pending transmission */
void send_message_ble_done(void)
{
    GLOBAL_INT_STORE_AND_DISABLE
    if(*send_buf_ble_rd != '\0')
    {
        /* continue transfer, if more data ready */
        send_message_ble_trigger_send();
    }
    else
    {
        tx_ble_busy = false;
    }
    GLOBAL_INT_RESTORE
}

/* INTERNAL: start UART TX*/
void send_message_ble_trigger_send(void)
{
    tx_ble_busy = true;
    const uint8_t SEND_LEN = strlen((char*)send_buf_ble_rd);
    R_Config_SCI1_BLE_Serial_Send((uint8_t*)send_buf_ble_rd, SEND_LEN);
    send_buf_ble_rd += SEND_LEN;
    /* read pointer roll over */
    if(send_buf_ble_rd >= send_buf_ble + TX_BUF_BLE - 1)
    {
        send_buf_ble_rd = send_buf_ble;
    }
}

/* buffer incoming messages */
void pass_message_ble(uint8_t const * const data, uint8_t const len)
{
    if (process_buffer_ble[0] == '\0')
    {
        /* buffer is empty / not in use -> store message */
        strncpy((char*)process_buffer_ble, (char*)data, len);
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
        log((uint8_t*)process_buffer_ble);  // TODO flo: debug remove
        /* last step: free buffer */
        memset((uint8_t*)process_buffer_ble, '\0', RX_BUF_BLE);
    }
}