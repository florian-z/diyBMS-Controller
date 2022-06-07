#include "main.h"
#include <string.h>
#include "log_util.h"

#include "uart_display.h"
#include "uart_usb.h"
#include "uart_cellmodule.h"
#include "cellmodule_data.h"
#include "pcf8574_pwr.h"
#include "shunt.h"

void charger_logic(void);
void led_test(void);
void config_communication(void);
void register_nmi_interrupt_handler(void);

#define MAINTAIN_WATCHDOG //R_Config_IWDT_Restart();
static volatile bool timer_tick = false;

#define CHARGER_LOAD_DELAY 2

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

    register_nmi_interrupt_handler();
    R_Config_TMR0_TMR1_Start(); // start timer tick
    shunt_init();

/// main loop
    uint8_t count_10ms = 0;
    log("Main Loop\n");
    bool toggle = false;
    for(;;)
    {
        MAINTAIN_WATCHDOG
        if (timer_tick)
        {
            timer_tick = false;
            count_10ms++;
            if (!(count_10ms % 10))
            {
                /* 10 Hz */
                tick_cellmodule();
            }

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

                    calc_cellmodule_data();
                    charger_logic();

                    //shunt_tick();

                    /// SPI shunt tests
                    //OUT_SPI_nMSS_ON
                    //OUT_SPI_nMSS_OFF
                    /// SPI shunt tests END



                    send_message_display("disp uart test\n");


                    if(toggle)
                    {
                        toggle=false;
                        //RELAIS_HEAT_ON
                        send_message_cellmodule("!0000*00\n");
                    }
                    else
                    {
                        toggle=true;
                        //RELAIS_HEAT_OFF
                        send_message_cellmodule("!0100*01\n");
                    }

                    print_cellmodule_full_debug();

                    //send_message_display();
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

void charger_logic()
{
static bool kl15_pwr_state = false;
static bool line_pwr_state = false;
static bool charger_active_state = false;
/// check line power active
    if (IN_SIGNAL_LINE_PWR)
    {
        // line power on
        if (!line_pwr_state)
        {
            // line power on latch
            line_pwr_state = true;
            log("LINE LATCH ON\n");
            // ensure balancer is on, when car is on
            RELAIS_BALANCER_ON
        }
        // check need for heating
        if (check_temp_should_use_heater())
        {
            RELAIS_HEAT_ON
            log("HEATER switch ON\n");
        } else {
            RELAIS_HEAT_OFF
            log("HEATER switch OFF\n");
        }
        // check if good for charging
        if (check_temp_charging_allowed() && check_volt_charging_necessary_start())
        {
            if (!charger_active_state)
            {
                log("CHARGE LATCH ON\n");
                OUT_CHARGER_LOAD_ON
                OUT_CHARGER_DOOR_ON
                charger_active_state = true;
            }
        }
        // check under/over-temp and charge-stop-voltage
        if (!check_temp_charging_allowed() || check_volt_charging_safety_stop())
        {
            OUT_CHARGER_DOOR_OFF
            if (charger_active_state)
            {
                log("CHARGE LATCH OFF\n");
                charger_active_state = false;
            } else {
                OUT_CHARGER_LOAD_OFF
            }
        }

    } else {
        // no line power
        if (line_pwr_state)
        {
            // line power off latch
            line_pwr_state = false;
            log("LINE LATCH OFF\n");
            charger_active_state = false;
            RELAIS_HEAT_OFF
            OUT_CHARGER_LOAD_OFF
            OUT_CHARGER_DOOR_OFF
        }
    }

/// check KL15 power active
    if (IN_SIGNAL_KL15_PWR)
    {
        // KL15 on
        if (!kl15_pwr_state)
        {
            // KL15 on latch
            log("KL15 LATCH ON\n");
            kl15_pwr_state = true;
            // ensure balancer is on, when car is on
            RELAIS_BALANCER_ON
        }
    } else {
        // no KL15
        if (kl15_pwr_state)
        {
            // KL15 off latch
            log("KL15 LATCH OFF\n");
            kl15_pwr_state = false;
        }
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
    MAINTAIN_WATCHDOG
    R_BSP_SoftwareDelay(500, BSP_DELAY_MILLISECS);
    LED_GN1_OFF
    LED_GN2_OFF
    LED_GE1_OFF
    LED_GE2_OFF
    LED_RT1_OFF
    LED_RT2_OFF
    LED_BL1_OFF
    LED_BL2_OFF
    MAINTAIN_WATCHDOG
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
    R_Config_SCI1_Display_Start();

    R_Config_RSPI0_Shunt_Start();
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
        MAINTAIN_WATCHDOG // TODO flo debug only, remove before flight
        R_BSP_SoftwareDelay(200, BSP_DELAY_MILLISECS);
    }
}



void nmi_interrupt_handler(void* args)
{
    #pragma diag_suppress=Pe177
    uint8_t reason = ((bsp_int_cb_args_t*)args)->vector;
    //    BSP_INT_SRC_EXC_SUPERVISOR_INSTR = 0, /* Occurs when privileged instruction is executed in User Mode */
    //    BSP_INT_SRC_EXC_UNDEFINED_INSTR,      /* Occurs when MCU encounters an unknown instruction */
    //    BSP_INT_SRC_EXC_NMI_PIN,              /* NMI Pin interrupt */
    //    BSP_INT_SRC_OSC_STOP_DETECT,          /* Oscillation stop is detected */
    //    BSP_INT_SRC_IWDT_ERROR,               /* IWDT underflow/refresh error has occurred */
    //    BSP_INT_SRC_LVD1,                     /* Voltage monitoring 1 interrupt */
    //    BSP_INT_SRC_LVD2,                     /* Voltage monitoring 2 interrupt */
    //    BSP_INT_SRC_UNDEFINED_INTERRUPT,      /* Interrupt has triggered for a vector that user did not write a handler. */
    //    BSP_INT_SRC_BUS_ERROR,                /* Bus error: illegal address access or timeout */
    //    BSP_INT_SRC_EMPTY
}

void register_nmi_interrupt_handler(void)
{
    R_BSP_InterruptWrite(BSP_INT_SRC_EXC_SUPERVISOR_INSTR, &nmi_interrupt_handler);
    R_BSP_InterruptWrite(BSP_INT_SRC_EXC_UNDEFINED_INSTR, &nmi_interrupt_handler);
    R_BSP_InterruptWrite(BSP_INT_SRC_EXC_NMI_PIN, &nmi_interrupt_handler);
    R_BSP_InterruptWrite(BSP_INT_SRC_OSC_STOP_DETECT, &nmi_interrupt_handler);
    R_BSP_InterruptWrite(BSP_INT_SRC_IWDT_ERROR, &nmi_interrupt_handler);
    R_BSP_InterruptWrite(BSP_INT_SRC_LVD1, &nmi_interrupt_handler);
    R_BSP_InterruptWrite(BSP_INT_SRC_LVD2, &nmi_interrupt_handler);
    R_BSP_InterruptWrite(BSP_INT_SRC_UNDEFINED_INTERRUPT, &nmi_interrupt_handler);
    R_BSP_InterruptWrite(BSP_INT_SRC_BUS_ERROR, &nmi_interrupt_handler);
    R_BSP_InterruptWrite(BSP_INT_SRC_EMPTY, &nmi_interrupt_handler);
}