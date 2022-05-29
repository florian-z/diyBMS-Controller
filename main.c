#include "main.h"
#include "display.h"
#include "uart_usb.h"
#include "cellmodule.h"
#include "pcf8574_pwr.h"
#include "log_util.h"

#include "string.h"

void led_test(void);
void config_communication(void);
void test_i2c_pwr(void);

static volatile bool timer_tick = false;

int main(void)
{
    led_test();
    config_communication();
    log("Booting... :)\n");
    // set safe defaults for relais
    RELAIS_BALANCER_ON
    RELAIS_HEAT_OFF
    RELAIS_PWR_MCU_ON
    RELAIS_PWR_SHUNT_ON

    R_Config_TMR0_TMR1_Start(); // start timer tick

    uint8_t count_10ms = 0;
    log("Main Loop\n");
    for(;;)
    {
        R_Config_IWDT_Restart();
        if (timer_tick)
        {
            timer_tick = false;
            count_10ms++;

/// GPIO tests done
            if (IN_SIGNAL_LINE_PWR)
            {
                // line power on
            } else {
                // no line power
            }

            if (IN_SIGNAL_KL15_PWR)
            {
                // KL15 on
            } else {
                // no KL15
            }

//            OUT_CHARGER_LOAD_ON
//            OUT_CHARGER_LOAD_OFF
//            OUT_CHARGER_DOOR_ON
//            OUT_CHARGER_DOOR_OFF
/// GPIO tests done END

            if (!(count_10ms % 25))
            {
                /* 4 Hz */
                LED_GE1_TGL
                send_message_pwr();


                if (count_10ms >= 100)
                {
                    /* 1 Hz */
                    count_10ms = 0;
                    LED_GE2_TGL

                    /// SPI shunt tests
                    //OUT_SPI_nMSS_ON
                    //OUT_SPI_nMSS_OFF
                    /// SPI shunt tests END


                    send_message_cellmodule("!0100*01\n");

                    send_message_display();
                }
                else
                {
                    /* 4 Hz, but only triggers if 1 Hz part is not executed */
                }
            }

        }

        process_message_usb();
        process_message_cellmodule();
        process_message_display();
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


static volatile uint8_t rx_buf_sci1_display[RX_BUF_DISPLAY] = {0};
static volatile uint8_t tx_buf_sci1_display[TX_BUF_DISPLAY] = {0};
void config_communication(void)
{
    R_Config_SCI6_USB_Start();

    R_Config_RIIC0_PWR_Start();

    R_Config_SCI0_CellModule_Start();
    R_Config_SCI5_CellModule_Start();
    //R_Config_SCI8_CellModule_Start();
    //R_Config_SCI9_CellModule_Start();

//    R_Config_SCI1_Display_Serial_Receive((uint8_t*)rx_buf_sci1_display, RX_BUF_DISPLAY);
//    R_Config_SCI1_Display_Start();
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
