#ifndef __process_message_h__
#define __process_message_h__

#include "main.h"
#include "r_cg_userdefine.h"

void send_message_cellmodule(void);
void send_message_display(void);

#define PCA8574_IDLE                (0)    /* all outputs inactive */
#define PCA8574_P0_HEAT_ON          (1<<0) /* turn mains heater on */
#define PCA8574_P1_HEAT_OFF         (1<<1) /* turn mains heater off */
#define PCA8574_P2_BAL_ON           (1<<2) /* turn balancer on */
#define PCA8574_P3_BAL_OFF          (1<<3) /* turn balancer off */
#define PCA8574_P4_PWR_SHUNT_ON     (1<<4) /* power supply for shunt on */
#define PCA8574_P5_PWR_SHUNT_OFF    (1<<5) /* power supply for shunt off */
#define PCA8574_P6_PWR_MCU_ON       (1<<6) /* power supply for mcu on */
#define PCA8574_P7_PWR_MCU_OFF      (1<<7) /* power supply for mcu off */
void config_message_pwr(uint8_t const flags);
void send_message_pwr();

void send_message_usb(uint8_t const * const data);
void send_message_usb_done(void);

void pass_message_usb(uint8_t const * const data, uint8_t const len);
void pass_message_cellmodule(uint8_t const * const data, uint8_t const len, uint8_t const chain_no);
void pass_message_display(uint8_t const * const data, uint8_t const len);

void process_message_usb();
void process_message_cellmodule();
void process_message_display();

#endif /* __process_message_h__ */