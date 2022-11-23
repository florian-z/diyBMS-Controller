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
#include "charger_logic.h"


void led_test(void);
void config_communication(void);
void register_nmi_interrupt_handler(void);
void tick_system_status(void);

#define MAINTAIN_WATCHDOG   R_Config_IWDT_Restart();
static volatile bool timer_tick = false;
static uint8_t capture_full_freeze_frame = 10;
uint8_t capture_compact_freeze_frame = 0;

// LED_GN1 on while message active on cell chain 1
// LED_GE1 on while uart-BLE-tx active
// LED_RT1 on for 2 seconds from systemstatus being too late / watchdog
// LED_BL1 on while comm with shunt active
// LED_GN2 on while message active on cell chain 1
// LED_GE2 on while uart-BLE-rx active
// LED_RT2 main loop 0.1Hz toggle
// LED_BL2 main loop on while active/busy

// LED_RT1 & LED_RT2 alternating fast -> Error_Handler()

int main(void)
{
    MAINTAIN_WATCHDOG
    led_test();
    config_communication();
    log("Booting... :)\n");

    // set safe defaults for relais
    OUT_CHARGER_DOOR_OFF
    OUT_CHARGER_LOAD_OFF
    OUT_HEATER_LATCH_OFF_CURR
    OUT_BAL_LATCH_OFF_CURR
    MAINTAIN_WATCHDOG
    R_BSP_SoftwareDelay(50, BSP_DELAY_MILLISECS); // 10ms needed for relais
    OUT_HEATER_LATCH_OFF_IDLE
    OUT_BAL_LATCH_OFF_IDLE

    register_nmi_interrupt_handler();
    R_Config_TMR0_TMR1_Start(); // start timer tick
    MAINTAIN_WATCHDOG
    shunt_init();
    //bluetooth_init_config_mode();
    bluetooth_init_run_mode();

/// main loop
    MAINTAIN_WATCHDOG
    uint8_t count_10ms = 0;
    uint8_t count_1sec = 0;
    log("Main Loop\n");
    for(;;)
    {
        // todo flo powersave / sleep
        LED_BL2_OFF
        if (timer_tick)
        {
            LED_BL2_ON
            timer_tick = false;
            count_10ms++;
            if (!(count_10ms % 5))
            {
                /* 20 Hz */
                shunt_tick();
            }
            if (!(count_10ms % 10))
            {
                /* 10 Hz */
                tick_system_status();
                tick_cellmodule();
                send_ble_android();
            }

            if (!(count_10ms % 25))
            {
                /* 4 Hz */


                if (count_10ms >= 100)
                {
                    /* 1 Hz */
                    count_10ms = 0;
                    count_1sec++;
                    time_tick_1sec();
                    report_system_status(MAIN_LOOP);

                    if (count_1sec%2)
                    {
                        send_message_cellmodule("!0000*00\n"); // u_batt_mv 0.5Hz
                    }
                    else
                    {
                        //log_va("time %s\n", get_ts_str());
                        send_message_cellmodule("!0100*01\n"); // temp_c 0.5Hz
                    }
                    calc_cellmodule_data();
                    charger_logic_tick();



                    if (!(count_1sec%10))
                    {
                        count_1sec = 0;
                        //log_cellmodule_full_debug();
                        //log_shunt_full_debug();
                        LED_RT2_TGL
                    }

                    if (capture_full_freeze_frame)
                    {
                        capture_full_freeze_frame--;
                        if(!capture_full_freeze_frame)
                        {
                            freezeframe_cellmodule_full_debug();
                            freezeframe_shunt_full_debug();
                        }
                    }
                    if (capture_compact_freeze_frame)
                    {
                        capture_compact_freeze_frame--;
                        if(!capture_compact_freeze_frame)
                        {
                            freezeframe_cellmodule_compact_debug();
                            freezeframe_shunt_full_debug();
                        }
                    }



                    //bluetooth_init_run_mode();
                    //send_ble_cmd(Read_Local_Info_0x01);
                    //send_message_ble_ascii("disp uart test\n");
                }
            }
        }

        process_message_usb();
        process_message_cellmodule();
        process_message_ble();
        process_message_shunt();
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

static uint8_t tss_cellmodule_chain1 = 0;
static uint8_t tss_cellmodule_chain2 = 0;
static uint8_t tss_shunt = 0;
static uint8_t tss_main_loop = 0;
// SYSTEM_STATUS_TICK_LIMIT
#define TSS_LIMIT 25
void tick_system_status(void)
{
    static uint8_t watchdog_led_on = 0;
    if (tss_cellmodule_chain1 < TSS_LIMIT) // prevent rollover
    {
        tss_cellmodule_chain1++;
    }
    else
    {
        log("TSS CHAIN1\n");
    }
    if (tss_cellmodule_chain2 < TSS_LIMIT) // prevent rollover
    {
        tss_cellmodule_chain2++;
    }
    else
    {
        log("TSS CHAIN2\n");
    }
    if (tss_shunt < TSS_LIMIT) // prevent rollover
    {
        tss_shunt++;
    }
    else
    {
        log("TSS SHUNT\n");
    }
    if (tss_main_loop < TSS_LIMIT) // prevent rollover
    {
        tss_main_loop++;
    }
    else
    {
        log("TSS MAIN LOOP\n");
    }

    if(tss_cellmodule_chain1 < TSS_LIMIT && tss_cellmodule_chain2 < TSS_LIMIT && tss_shunt < TSS_LIMIT && tss_main_loop < TSS_LIMIT)
    {
        // all supervised functions are reporting success in a timely manner
        MAINTAIN_WATCHDOG
    }
    else
    {
        watchdog_led_on = 21;
    }

    if (watchdog_led_on)
    {
        LED_RT1_ON
        watchdog_led_on--;
    }
    else
    {
        LED_RT1_OFF
    }

}

void report_system_status(enum system_status id)
{
    switch(id)
    {
        case CELLMODULE_CHAIN1:
            tss_cellmodule_chain1 = 0;
            break;
        case CELLMODULE_CHAIN2:
            tss_cellmodule_chain2 = 0;
            break;
        case SHUNT:
            tss_shunt = 0;
            break;
        case MAIN_LOOP:
            tss_main_loop = 0;
            break;
    }
}