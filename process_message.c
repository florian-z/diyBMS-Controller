#include "process_message.h"
#include "string.h"

static volatile uint8_t process_buffer_usb[RX_BUF_USB] = {0};

/* buffer incoming messages */
void pass_message_usb(uint8_t const * const data, uint8_t const len)
{
    if (process_buffer_usb[0] == '\0')
    {
        /* buffer is empty / not in use */
        strncpy((char*)process_buffer_usb, (char*)data, len);
    }
    else
    {
        Error_Handler();
    }
}


void pass_message_cellmodule(uint8_t const * const data, uint8_t const len, uint8_t const chain_no)
{

}

void pass_message_display(uint8_t const * const data, uint8_t const len)
{

}

/* process buffered messages */
void process_message_usb()
{
    if (process_buffer_usb[0] == '\0')
    {
        /* buffer is not empty -> process message */
        R_Config_SCI6_USB_Send_Copy((uint8_t*)process_buffer_usb);
        /* last step: free buffer */
        memset((uint8_t*)process_buffer_usb, '\0', RX_BUF_USB);
    }
}

void process_message_cell_module()
{

}

void process_message_display()
{

}