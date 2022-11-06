#include "main.h"
#include <string.h>
#include "log_util.h"
#include "freeze_util.h"
#include "time_util.h"

#include "uart_ble.h"
#include "uart_usb.h"
#include "uart_cellmodule.h"
#include "cellmodule_data.h"
#include "bluetooth.h"
#include "shunt.h"


void charger_logic(void);
void led_test(void);
void config_communication(void);
void register_nmi_interrupt_handler(void);

#define MAINTAIN_WATCHDOG //R_Config_IWDT_Restart();
static volatile bool timer_tick = false;

// LED_GN1 on while message active on cell chain 1
// LED_GE1 on while uart-BLE-tx active
// LED_RT1
// LED_BL1 on while comm with shunt active
// LED_GN2 on while message active on cell chain 1
// LED_GE2 on while uart-BLE-rx active
// LED_RT2
// LED_BL2 main loop

// LED_RT1 & LED_RT2 alternating fast -> Error_Handler()

int main(void)
{
    led_test();
    config_communication();
    log("Booting... :)\n");

    // set safe defaults for relais
    OUT_CHARGER_DOOR_OFF
    OUT_CHARGER_LOAD_OFF
    OUT_HEATER_LATCH_OFF_CURR
    OUT_BAL_LATCH_OFF_CURR
    MAINTAIN_WATCHDOG
    R_BSP_SoftwareDelay(50, BSP_DELAY_MILLISECS);
    OUT_HEATER_LATCH_OFF_IDLE
    OUT_BAL_LATCH_OFF_IDLE

    register_nmi_interrupt_handler();
    R_Config_TMR0_TMR1_Start(); // start timer tick
    shunt_init();
    //bluetooth_init_config_mode();
    bluetooth_init_run_mode();
    //send_ble_cmd(Read_Local_Info_0x01);

/// main loop
    uint8_t count_10ms = 0;
    uint8_t count_1sec = 0;
    log("Main Loop\n");
    for(;;)
    {
        MAINTAIN_WATCHDOG
        // todo flo powersave / sleep
        LED_BL2_OFF
        if (timer_tick)
        {
            LED_BL2_ON
            timer_tick = false;
            count_10ms++;
            if (!(count_10ms % 10))
            {
                /* 10 Hz */
                tick_cellmodule();
                shunt_tick();
            }

            if (!(count_10ms % 25))
            {
                /* 4 Hz */
                //send_message_pwr_tick();


                if (count_10ms >= 100)
                {
                    /* 1 Hz */
                    count_10ms = 0;
                    count_1sec++;
                    time_tick_1sec();

                    if (count_1sec%2)
                    {
                        send_message_cellmodule("!0000*00\n"); // u_batt_mv 0.5Hz
                    }
                    else
                    {
                        send_message_cellmodule("!0100*01\n"); // temp_c 0.5Hz
                    }
                    calc_cellmodule_data();
                    charger_logic();

                    log_va("time %s\n", get_ts_str());
                    freeze_va("%02i jo test jojo, sag au\n", count_10ms); // TODO remove

                    if (count_1sec >= 9)
                    {
                        count_1sec = 0;
//                        print_cellmodule_full_debug();
                        print_shunt_full_debug();
                        LED_RT2_TGL
                    }

                    //bluetooth_init_run_mode();
                    //send_ble_cmd(Read_Local_Info_0x01);
                    //send_message_ble("disp uart test\n");
                }
            }
        }

        process_message_usb();
        process_message_cellmodule();
        process_message_ble();
        process_message_shunt();
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
/// ensure relais IDLE is reached
    OUT_BAL_LATCH_OFF_IDLE
    OUT_BAL_LATCH_ON_IDLE
    OUT_HEATER_LATCH_OFF_IDLE
    OUT_HEATER_LATCH_ON_IDLE
/// check line power active
    if (IN_SIGNAL_LINE_PWR)
    {
        // line power on
        if (!line_pwr_state)
        {
            // line power on latch
            log("LINE DETECT LATCH ON:BAL ON\n");
            line_pwr_state = true;
            // ensure balancer is on, when car is on
            OUT_BAL_LATCH_OFF_IDLE
            OUT_BAL_LATCH_ON_CURR
        }
        // check need for heating
        bool check_temp_should_use_heater_var = check_temp_should_use_heater();
        bool check_age_ticks_u_batt_and_temp_allowed_var = check_age_ticks_u_batt_and_temp_allowed();
        if (check_temp_should_use_heater_var && check_age_ticks_u_batt_and_temp_allowed_var)
        {
            if (!heater_active_state)
            {
                log("HEATER LATCH ON\n");
                OUT_HEATER_LATCH_OFF_IDLE
                OUT_HEATER_LATCH_ON_CURR
                heater_active_state = true;
            }
        } else {
            if (heater_active_state)
            {
                log("HEATER LATCH OFF");
                if(!check_temp_should_use_heater_var) {
                    log(":REACHED TEMP");
                }
                if (!check_age_ticks_u_batt_and_temp_allowed_var) {
                    log(":CELL DATA TO OLD");
                }
                log("\n");
                OUT_HEATER_LATCH_ON_IDLE
                OUT_HEATER_LATCH_OFF_CURR
                heater_active_state = false;
            }
        }
        // check if good for charging
        bool check_temp_charging_allowed_var = check_temp_charging_allowed();
        bool check_volt_charging_necessary_start_var = check_volt_charging_necessary_start();
        //bool check_age_ticks_u_batt_and_temp_allowed_var = check_age_ticks_u_batt_and_temp_allowed();
        if (check_temp_charging_allowed_var && check_volt_charging_necessary_start_var && !check_age_ticks_u_batt_and_temp_allowed_var)
        {
            if (!charger_active_state)
            {
                log("CHARGE LATCH ON\n");
                OUT_CHARGER_LOAD_ON
                charger_active_state = true;
            }
            else
            {
                OUT_CHARGER_DOOR_ON
            }
        } else {
            log("NOT READY TO CHG");
            if(!check_temp_charging_allowed_var) {
                log(":TEMP DOES NOT ALLOW");
            }
            if(!check_volt_charging_necessary_start_var) {
                log(":VOLT CHG NOT NEEDED");
            }
            if(!check_temp_charging_allowed_var) {
                log(":CELL DATA TO OLD");
            }
            log("\n");
        }
        // check under/over-temp and charge-stop-voltage
        //bool check_temp_charging_allowed_var = check_temp_charging_allowed();
        bool check_volt_charging_safety_stop_var = check_volt_charging_safety_stop();
        //bool check_age_ticks_u_batt_and_temp_allowed_var = check_age_ticks_u_batt_and_temp_allowed();
        if (!check_temp_charging_allowed_var || check_volt_charging_safety_stop_var || !check_age_ticks_u_batt_and_temp_allowed_var)
        {
            OUT_CHARGER_DOOR_OFF
            if (charger_active_state)
            {
                log("CHARGE LATCH OFF");
                if(!check_temp_charging_allowed_var) {
                    log(":TEMP DOES NOT ALLOW");
                }
                if (check_volt_charging_safety_stop_var) {
                    log(":VOLT SAFETY STOP");
                }
                if (!check_age_ticks_u_batt_and_temp_allowed_var) {
                    log(":CELL DATA TO OLD");
                }
                log("\n");
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
            log("LINE DETECT LATCH OFF:HEATER OFF:CHG LOAD OFF:CHG DOOR OFF\n");
            charger_active_state = false;
            OUT_HEATER_LATCH_ON_IDLE
            OUT_HEATER_LATCH_OFF_CURR
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
            log("KL15 DETECT LATCH ON:BAL ON\n");
            kl15_pwr_state = true;
            // ensure balancer is on, when car is on
            OUT_BAL_LATCH_OFF_IDLE
            OUT_BAL_LATCH_ON_CURR
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