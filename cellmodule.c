#include "cellmodule.h"
#include "string.h"

#define CELLMODULE_CHANNELS 1

/*** CellModules UART ***/
static volatile uint8_t process_buffer_cellmodule[CELLMODULE_CHANNELS][RX_BUF_CELLMODULE] = {0};
/* generate and send messages */
void send_message_cellmodule(uint8_t const * const data)
{

}

/* called when transmission done, could check for further pending transmission */
void send_message_cellmodule_done(uint8_t const chain_no)
{

}

/* buffer incoming messages */
void pass_message_cellmodule(uint8_t const * const data, uint8_t const len, uint8_t const chain_no)
{
    if (process_buffer_cellmodule[chain_no][0] == '\0')
    {
        /* buffer is empty / not in use -> store message */
        strncpy((char*)process_buffer_cellmodule[chain_no], (char*)data, len);
    }
    else
    {
        Error_Handler();
    }
}

/* process buffered incoming messages */
void process_message_cellmodule()
{
    for(uint8_t i = 0; i < CELLMODULE_CHANNELS; i++)
    {
        if (process_buffer_cellmodule[i][0] == '\0')
        {
            /* buffer is not empty -> process message */
            send_message_usb((uint8_t*)process_buffer_cellmodule[i]);  // TODO flo: debug remove
            /* last step: free buffer */
            memset((uint8_t*)process_buffer_cellmodule[i], '\0', RX_BUF_CELLMODULE);
        }
    }
}
