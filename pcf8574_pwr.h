#ifndef __pcf8574_pwr_h__
#define __pcf8574_pwr_h__

#include "main.h"
#include "r_cg_userdefine.h"

#define PCA8574_IDLE                (0)    /* all outputs inactive */
#define PCA8574_P0_HEAT_ON          (1<<0) /* turn mains heater on */
#define PCA8574_P1_HEAT_OFF         (1<<1) /* turn mains heater off */
#define PCA8574_P2_BAL_ON           (1<<2) /* turn balancer on */
#define PCA8574_P3_BAL_OFF          (1<<3) /* turn balancer off */
#define PCA8574_P4_PWR_SHUNT_ON     (1<<4) /* power supply for shunt on */
#define PCA8574_P5_PWR_SHUNT_OFF    (1<<5) /* power supply for shunt off */
#define PCA8574_P6_PWR_MCU_ON       (1<<6) /* power supply for mcu on */
#define PCA8574_P7_PWR_MCU_OFF      (1<<7) /* power supply for mcu off */

#define RELAIS_HEAT_ON          config_message_pwr(PCA8574_P0_HEAT_ON);
#define RELAIS_HEAT_OFF         config_message_pwr(PCA8574_P1_HEAT_OFF);
#define RELAIS_BALANCER_ON      config_message_pwr(PCA8574_P2_BAL_ON);
#define RELAIS_BALANCER_OFF     config_message_pwr(PCA8574_P3_BAL_OFF);


void config_message_pwr(uint8_t const flags);
void send_message_pwr_tick();

#endif /* __pcf8574_pwr_h__ */