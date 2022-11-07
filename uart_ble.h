#ifndef __uart_display_h__
#define __uart_display_h__

#include "main.h"
#include "r_cg_userdefine.h"

void send_message_ble_binary(uint8_t const * const data, uint8_t const data_len);
void send_message_ble_ascii(uint8_t const * const data);
void send_message_ble_done(void);

void pass_message_ble(uint8_t const * const data, uint8_t const len);
void process_message_ble();

#endif /* __uart_display_h__ */