#ifndef __uart_usb_h__
#define __uart_usb_h__

#include "main.h"
#include "r_cg_userdefine.h"

void send_message_usb(uint8_t const * const data);
void send_message_usb_done(void);
void send_message_usb_no_buffer(uint8_t const * const send_data, uint16_t const send_len);

void pass_message_usb(uint8_t const * const data, uint8_t const len);
void process_message_usb();

#endif /* __uart_usb_h__ */