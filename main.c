#include "main.h"
#include <string.h>
#include "log_util.h"

#include "uart_ble.h"
#include "uart_usb.h"
#include "uart_cellmodule.h"
#include "cellmodule_data.h"
#include "bluetooth.h"
#include "shunt.h"
#include "pcf8574_pwr.h"

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

    register_nmi_interrupt_handler();
    R_Config_TMR0_TMR1_Start(); // start timer tick
    shunt_init();
    bluetooth_init();

/// main loop
    uint8_t count_10ms = 0;
    log("Main Loop\n");
    for(;;)
    {
        MAINTAIN_WATCHDOG
        // todo flo powersave / sleep
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
                send_message_pwr_tick();


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



                    send_message_ble("disp uart test\n");

                    print_cellmodule_full_debug();
                    print_shunt_full_debug();




                    send_message_cellmodule("!0100*01\n"); // temp_c 1Hz
                }
                else
                {
                    /* 4 Hz, but only triggers if 1 Hz part is not executed */
                    send_message_cellmodule("!0000*00\n"); // u_batt_mv ca. 3Hz
                }
            }

        }

        process_message_usb();
        process_message_cellmodule();
        process_message_ble();
    }
}

// true if charging (LINE DETECT) or key-on (KL15 ON)
static bool car_active = false;
void charger_logic()
{
    static bool kl15_pwr_state = false;
    static bool line_pwr_state = false;
    static bool heater_active_state = false;
    static bool charger_active_state = false;
/// check line power active
    if (IN_SIGNAL_LINE_PWR)
    {
        // line power on
        if (!line_pwr_state)
        {
            // line power on latch
            log("LINE DETECT LATCH ON\n");
            line_pwr_state = true;
            // ensure balancer is on, when car is on
            RELAIS_BALANCER_ON
        }
        // check need for heating
        if (check_temp_should_use_heater() && check_age_ticks_u_batt_and_temp_allowed())
        {
            if (!heater_active_state)
            {
                log("HEATER LATCH ON\n");
                RELAIS_HEAT_ON
                heater_active_state = true;
            }
        } else {
            if (heater_active_state)
            {
                log("HEATER LATCH OFF\n");
                RELAIS_HEAT_OFF
                heater_active_state = false;
            }
        }
        // check if good for charging
        if (check_temp_charging_allowed() && check_volt_charging_necessary_start() && !check_age_ticks_u_batt_and_temp_allowed())
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
        if (!check_temp_charging_allowed() || check_volt_charging_safety_stop() || !check_age_ticks_u_batt_and_temp_allowed())
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
            log("LINE DETECT LATCH OFF\n");
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
            log("KL15 DETECT LATCH ON\n");
            kl15_pwr_state = true;
            // ensure balancer is on, when car is on
            RELAIS_BALANCER_ON
        }
    } else {
        // no KL15
        if (kl15_pwr_state)
        {
            // KL15 off latch
            log("KL15 DETECT LATCH OFF\n");
            kl15_pwr_state = false;
        }
    }

/// check if car is sleeping or not
    if (kl15_pwr_state || line_pwr_state)
    {
        // car active (KL15 ON and / or LINE DETECT)
        if (!car_active)
        {
            log("CAR ACTIVE LATCH ON\n");
            car_active = true;
        }
    } else {
        // car sleeping
        if (car_active)
        {
            log("CAR ACTIVE LATCH OFF\n");
            car_active = false;
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


void config_communication(void)
{
    R_Config_SCI6_USB_Start();

    R_Config_RIIC0_Start();

    R_Config_SCI0_CellModule_Start();
    R_Config_SCI5_CellModule_Start();
    //R_Config_SCI8_CellModule_Start();
    //R_Config_SCI9_CellModule_Start();

    R_Config_SCI1_BLE_Start();

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