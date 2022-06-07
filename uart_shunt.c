#include "uart_shunt.h"
#include <string.h>
#include <stdlib.h>

#include "log_util.h"


/*** Shunt UART ***/
static volatile uint8_t send_buf_shunt[TX_BUF_SHUNT] = {0};     /* transmit buffer */
static volatile uint8_t* send_buf_shunt_wr = send_buf_shunt;    /* write pointer */
static volatile uint8_t* send_buf_shunt_rd = send_buf_shunt;    /* read pointer */
static volatile bool tx_shunt_busy = false;                   /* transmit active */
void send_message_shunt_trigger_send(void);

static volatile uint8_t process_buffer_shunt[RX_BUF_SHUNT] = {0};   /* receive buffer*/

/* generate and send messages */
void send_message_shunt(uint8_t const * const data)
{
    GLOBAL_INT_STORE_AND_DISABLE
    const uint8_t DATA_LEN = strlen((char*)data);
    for(uint8_t i = 0; i<DATA_LEN; i++)
    {
        /* ensure no data gets overwritten */
        if(*send_buf_shunt_wr != '\0')
        {
            /* buffer full -> skip */
            break;
        }
        *send_buf_shunt_wr++ = data[i];
        /* write pointer roll over */
        if(send_buf_shunt_wr >= send_buf_shunt + TX_BUF_SHUNT - 1)
        {
            send_buf_shunt_wr = send_buf_shunt;
        }
    }
    /* start transmission */
    if(!tx_shunt_busy)
    {
        send_message_shunt_trigger_send();
    }
    GLOBAL_INT_RESTORE
}

/* called when ready for next transmission, checks for further pending transmission */
void send_message_shunt_done(void)
{
    GLOBAL_INT_STORE_AND_DISABLE
    if(*send_buf_shunt_rd != '\0')
    {
        /* continue transfer, if more data ready */
        send_message_shunt_trigger_send();
    }
    else
    {
        tx_shunt_busy = false;
    }
    GLOBAL_INT_RESTORE
}

/* INTERNAL: start UART TX*/
void send_message_shunt_trigger_send(void)
{
    tx_shunt_busy = true;
    const uint8_t SEND_LEN = strlen((char*)send_buf_shunt_rd);
    // TODO flo abc-todo-R_Config_SCI1_Display_Serial_Send((uint8_t*)send_buf_shunt_rd, SEND_LEN);
    send_buf_shunt_rd += SEND_LEN;
    /* read pointer roll over */
    if(send_buf_shunt_rd >= send_buf_shunt + TX_BUF_SHUNT - 1)
    {
        send_buf_shunt_rd = send_buf_shunt;
    }
}

/* buffer incoming messages */
void pass_message_shunt(uint8_t const * const data, uint8_t const len)
{
    if (process_buffer_shunt[0] == '\0')
    {
        /* buffer is empty / not in use -> store message */
        strncpy((char*)process_buffer_shunt, (char*)data, len);
    }
    else
    {
        Error_Handler();
    }
}

/* process buffered incoming messages */
void process_message_shunt()
{
    if (process_buffer_shunt[0] != '\0')
    {
        /* buffer is not empty -> process message */
        log((uint8_t*)process_buffer_shunt);  // TODO flo: debug remove
        /* last step: free buffer */
        memset((uint8_t*)process_buffer_shunt, '\0', RX_BUF_SHUNT);
    }
}