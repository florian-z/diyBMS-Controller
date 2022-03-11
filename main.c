#include "main.h"
#include "process_message.h"

void led_test(void);
void config_communication(void);

static volatile bool timer_tick = false;

int main(void)
{
    led_test();
    config_communication();
    R_Config_TMR0_TMR1_Start(); // start timer tick

    for(;;)
    {
        R_Config_IWDT_Restart();
        R_BSP_SoftwareDelay(1000, BSP_DELAY_MILLISECS);
        if (timer_tick)
        {
            LED_GE1_TGL
        }

        uint8_t* test_msg = "Hi, Testnachricht :)\n";
        R_Config_SCI6_USB_Send_Copy(test_msg);

        process_message_usb();
    }
}


void led_test(void)
{
    LED_GN1_ON
    LED_GN2_ON
    LED_GE1_ON
    LED_GE2_ON
    LED_RT1_ON
    LED_RT2_ON
    LED_BL1_ON
    LED_BL2_ON
    R_Config_IWDT_Restart();
    R_BSP_SoftwareDelay(500, BSP_DELAY_MILLISECS);
    LED_GN1_OFF
    LED_GN2_OFF
    LED_GE1_OFF
    LED_GE2_OFF
    LED_RT1_OFF
    LED_RT2_OFF
    LED_BL1_OFF
    LED_BL2_OFF
    R_Config_IWDT_Restart();
    R_BSP_SoftwareDelay(200, BSP_DELAY_MILLISECS);
}


//static volatile uint8_t rx_buf_sci6_usb[RX_BUF_USB] = {0};
static volatile uint8_t rx_buf_sci0_cellmodule[RX_BUF_CELLMODULE] = {0};
static volatile uint8_t rx_buf_sci5_cellmodule[RX_BUF_CELLMODULE] = {0};
static volatile uint8_t rx_buf_sci8_cellmodule[RX_BUF_CELLMODULE] = {0};
static volatile uint8_t rx_buf_sci9_cellmodule[RX_BUF_CELLMODULE] = {0};
static volatile uint8_t rx_buf_sci1_display[RX_BUF_DISPLAY] = {0};

//static volatile uint8_t tx_buf_sci6_usb[TX_BUF_USB] = {0};
static volatile uint8_t tx_buf_sci0_cellmodule[TX_BUF_CELLMODULE] = {0};
static volatile uint8_t tx_buf_sci5_cellmodule[TX_BUF_CELLMODULE] = {0};
static volatile uint8_t tx_buf_sci8_cellmodule[TX_BUF_CELLMODULE] = {0};
static volatile uint8_t tx_buf_sci9_cellmodule[TX_BUF_CELLMODULE] = {0};
static volatile uint8_t tx_buf_sci1_display[TX_BUF_DISPLAY] = {0};
void config_communication(void)
{
//    R_Config_SCI6_USB_Serial_Receive(rx_buf_sci6_usb, RX_BUF_USB);
    R_Config_SCI6_USB_Start();

//    R_Config_SCI0_CellModule_Serial_Receive(rx_buf_sci0_cellmodule, RX_BUF_CELLMODULE);
    R_Config_SCI0_CellModule_Start();
    R_Config_SCI5_CellModule_Serial_Receive((uint8_t*)rx_buf_sci5_cellmodule, RX_BUF_CELLMODULE);
    R_Config_SCI5_CellModule_Start();
    R_Config_SCI8_CellModule_Serial_Receive((uint8_t*)rx_buf_sci8_cellmodule, RX_BUF_CELLMODULE);
    R_Config_SCI8_CellModule_Start();
    R_Config_SCI9_CellModule_Serial_Receive((uint8_t*)rx_buf_sci9_cellmodule, RX_BUF_CELLMODULE);
    R_Config_SCI9_CellModule_Start();

    R_Config_SCI1_Display_Serial_Receive((uint8_t*)rx_buf_sci1_display, RX_BUF_DISPLAY);
    R_Config_SCI1_Display_Start();
}

void main_timer_tick(void)
{
    timer_tick = true;
}

void Error_Handler(void)
{
    __disable_interrupt();
    LED_RT1_OFF
    LED_RT2_ON
    for(;;)
    {
        LED_RT1_TGL
        LED_RT2_TGL
        R_Config_IWDT_Restart();
        R_BSP_SoftwareDelay(200, BSP_DELAY_MILLISECS);
    }
}