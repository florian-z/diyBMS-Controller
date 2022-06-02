#include "uart_display.h"
#include "string.h"
#include "stdlib.h"

#include "log_util.h"


/*** Display UART ***/
static volatile uint8_t send_buf_display[TX_BUF_DISPLAY] = {0};     /* transmit buffer */
static volatile uint8_t* send_buf_display_wr = send_buf_display;    /* write pointer */
static volatile uint8_t* send_buf_display_rd = send_buf_display;    /* read pointer */
static volatile bool tx_display_busy = false;                   /* transmit active */
void send_message_display_trigger_send(void);

static volatile uint8_t process_buffer_display[RX_BUF_DISPLAY] = {0};   /* receive buffer*/

/* generate and send messages */
void send_message_display(uint8_t const * const data)
{
    GLOBAL_INT_STORE_AND_DISABLE
    const uint8_t DATA_LEN = strlen((char*)data);
    for(uint8_t i = 0; i<DATA_LEN; i++)
    {
        /* ensure no data gets overwritten */
        if(*send_buf_display_wr != '\0')
        {
            /* buffer full -> skip */
            break;
        }
        *send_buf_display_wr++ = data[i];
        /* write pointer roll over */
        if(send_buf_display_wr >= send_buf_display + TX_BUF_DISPLAY - 1)
        {
            send_buf_display_wr = send_buf_display;
        }
    }
    /* start transmission */
    if(!tx_display_busy)
    {
        send_message_display_trigger_send();
    }
    GLOBAL_INT_RESTORE
}

/* called when ready for next transmission, checks for further pending transmission */
void send_message_display_done(void)
{
    GLOBAL_INT_STORE_AND_DISABLE
    if(*send_buf_display_rd != '\0')
    {
        /* continue transfer, if more data ready */
        send_message_display_trigger_send();
    }
    else
    {
        tx_display_busy = false;
    }
    GLOBAL_INT_RESTORE
}

/* INTERNAL: start UART TX*/
void send_message_display_trigger_send(void)
{
    tx_display_busy = true;
    const uint8_t SEND_LEN = strlen((char*)send_buf_display_rd);
    R_Config_SCI1_Display_Serial_Send((uint8_t*)send_buf_display_rd, SEND_LEN);
    send_buf_display_rd += SEND_LEN;
    /* read pointer roll over */
    if(send_buf_display_rd >= send_buf_display + TX_BUF_DISPLAY - 1)
    {
        send_buf_display_rd = send_buf_display;
    }
}

/* buffer incoming messages */
void pass_message_display(uint8_t const * const data, uint8_t const len)
{
    if (process_buffer_display[0] == '\0')
    {
        /* buffer is empty / not in use -> store message */
        strncpy((char*)process_buffer_display, (char*)data, len);
    }
    else
    {
        Error_Handler();
    }
}

/* process buffered incoming messages */
void process_message_display()
{
    if (process_buffer_display[0] != '\0')
    {
        /* buffer is not empty -> process message */
        log((uint8_t*)process_buffer_display);  // TODO flo: debug remove
        /* last step: free buffer */
        memset((uint8_t*)process_buffer_display, '\0', RX_BUF_DISPLAY);
    }
}