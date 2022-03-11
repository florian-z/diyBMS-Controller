#include "process_message.h"
#include "string.h"


#define PCA8574_ADR 0x20    /* 7-bit address without read/write-flag */
#define CELLMODULE_CHANNELS 1


/*** PCA8574 I2C PWR ***/
/* PCA7584 all outputs are active low */
#define PCA8574_ALL_OUTPUTS_INACTIVE 0xFF
static volatile uint8_t pca8574_outputs = PCA8574_ALL_OUTPUTS_INACTIVE;
/* buffer commands */
void config_message_pwr(uint8_t const flags)
{
    GLOBAL_INT_STORE_AND_DISABLE
    /* OFF flags have prio over ON flags */
    if(PCA8574_P1_HEAT_OFF & flags) {
        pca8574_outputs &= ~PCA8574_P1_HEAT_OFF;
    } else if (PCA8574_P0_HEAT_ON & flags) {
        pca8574_outputs &= ~PCA8574_P0_HEAT_ON;
    }
    if(PCA8574_P3_BAL_OFF & flags) {
        pca8574_outputs &= ~PCA8574_P3_BAL_OFF;
    } else if (PCA8574_P2_BAL_ON & flags) {
        pca8574_outputs &= ~PCA8574_P2_BAL_ON;
    }
    if(PCA8574_P5_PWR_SHUNT_OFF & flags) {
        pca8574_outputs &= ~PCA8574_P5_PWR_SHUNT_OFF;
    } else if (PCA8574_P4_PWR_SHUNT_ON & flags) {
        pca8574_outputs &= ~PCA8574_P4_PWR_SHUNT_ON;
    }
    if(PCA8574_P7_PWR_MCU_OFF & flags) {
        pca8574_outputs &= ~PCA8574_P7_PWR_MCU_OFF;
    } else if (PCA8574_P6_PWR_MCU_ON & flags) {
        pca8574_outputs &= ~PCA8574_P6_PWR_MCU_ON;
    }
    GLOBAL_INT_RESTORE
}
/* send commands */
void send_message_pwr()
{
    GLOBAL_INT_STORE_AND_DISABLE
    static uint8_t pca8754_output_tx_buf = PCA8574_ALL_OUTPUTS_INACTIVE;
    pca8754_output_tx_buf = pca8574_outputs;
    pca8574_outputs = PCA8574_ALL_OUTPUTS_INACTIVE; // reset for next transfer
    GLOBAL_INT_RESTORE
    R_Config_RIIC0_PWR_Master_Send(PCA8574_ADR, &pca8754_output_tx_buf, 1);
}

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
}

/* called when transmission done, checks for further pending transmission */
void send_message_usb_done(void)
{
    if(*send_buf_usb_rd != '\0')
    {
        /* continue transfer, if more data ready */
        send_message_usb_trigger_send();
    }
    else
    {
        tx_usb_busy = false;
    }
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
    if (process_buffer_usb[0] == '\0')
    {
        /* buffer is not empty -> process message */
        R_Config_SCI6_USB_Send_Copy((uint8_t*)process_buffer_usb);  // TODO flo: debug remove
        /* last step: free buffer */
        memset((uint8_t*)process_buffer_usb, '\0', RX_BUF_USB);
    }
}

/*** CellModules UART ***/
static volatile uint8_t process_buffer_cellmodule[CELLMODULE_CHANNELS][RX_BUF_CELLMODULE] = {0};
/* generate and send messages */
void send_message_cellmodule(void)
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
            R_Config_SCI6_USB_Send_Copy((uint8_t*)process_buffer_cellmodule[i]);  // TODO flo: debug remove
            /* last step: free buffer */
            memset((uint8_t*)process_buffer_cellmodule[i], '\0', RX_BUF_CELLMODULE);
        }
    }
}

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
    if (process_buffer_display[0] == '\0')
    {
        /* buffer is not empty -> process message */
        R_Config_SCI6_USB_Send_Copy((uint8_t*)process_buffer_display);  // TODO flo: debug remove
        /* last step: free buffer */
        memset((uint8_t*)process_buffer_display, '\0', RX_BUF_DISPLAY);
    }
}