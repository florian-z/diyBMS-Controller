#ifndef __uart_shunt_h__
#define __uart_shunt_h__

#include "main.h"
#include "r_cg_userdefine.h"

void send_message_shunt(uint8_t const * const data);
void send_message_shunt_trigger_send(void);

void pass_message_shunt(uint8_t const * const data, uint8_t const len);
void process_message_shunt();

#endif /* __uart_shunt_h__ */