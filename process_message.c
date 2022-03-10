#include "process_message.h"

static volatile uint8_t process_buffer_usb[RX_BUF_USB] = {0};

void pass_message_usb(uint8_t const * const data, uint8_t const len)
{
    if (local_buffer[0] == '\0')
    {
        /* buffer is empty / not in use */
        strncpy(process_buffer_usb, data, len);
    }
    else
    {
        ErrorHandler();
    }
}

void pass_message_cell_module(uint8_t const * const data, uint8_t const len, uint8_t const chain_no)
{

}

void pass_message_display(uint8_t const * const data)
{

}

void process_message_usb()
{
    if (process_buffer_usb[0] == '\0')
    {
        /* buffer is not empty -> process message */
        R_Config_SCI6_USB_Send_Copy(process_buffer_usb);
        /* last step: free buffer */
        memset(local_buffer, '\0', RX_BUF_USB);
    }
}

void process_message_cell_module(uint8_t const * const data, uint8_t const chain_no)
{

}

void process_message_display(uint8_t const * const data)
{

}