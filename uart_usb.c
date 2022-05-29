#include "uart_usb.h"
#include "string.h"

/*** USB UART ***/
static volatile uint8_t send_buf_usb[TX_BUF_USB] = {0};     /* transmit buffer */
static volatile uint8_t* send_buf_usb_wr = send_buf_usb;    /* write pointer */
static volatile uint8_t* send_buf_usb_rd = send_buf_usb;    /* read pointer */
static volatile bool tx_usb_busy = false;                   /* transmit active */
static volatile uint8_t process_buffer_usb[RX_BUF_USB] = {0};
void send_message_usb_trigger_send(void);
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
            break;
        }
        *send_buf_usb_wr++ = data[i];
        /* write pointer roll over */
        if(send_buf_usb_wr >= send_buf_usb + TX_BUF_USB)
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

/* called when transmission done, checks for further pending transmission */
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
    const uint8_t SEND_LEN = strlen((char*)send_buf_usb_rd);
    R_Config_SCI6_USB_Serial_Send((uint8_t*)send_buf_usb_rd, SEND_LEN);
    send_buf_usb_rd += SEND_LEN;
    /* read pointer roll over */
    if(send_buf_usb_rd >= send_buf_usb + TX_BUF_USB)
    {
        send_buf_usb_rd = send_buf_usb;
    }
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
        send_message_usb((uint8_t*)process_buffer_usb);  // TODO flo: debug remove
        /* last step: free buffer */
        memset((uint8_t*)process_buffer_usb, '\0', RX_BUF_USB);
    }
}
