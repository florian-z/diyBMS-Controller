#include "display.h"
#include "string.h"

/*** Display UART ***/
static volatile uint8_t process_buffer_display[RX_BUF_DISPLAY] = {0};
/* generate and send messages */
void send_message_display(void)
{

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
        R_Config_SCI1_Display_Serial_Send((uint8_t*)process_buffer_display, strlen((char*)process_buffer_display));  // TODO flo: debug remove
        /* last step: free buffer */
        memset((uint8_t*)process_buffer_display, '\0', RX_BUF_DISPLAY);
    }
}