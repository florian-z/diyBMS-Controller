#ifndef __display_h__
#define __display_h__

#include "main.h"
#include "r_cg_userdefine.h"

void send_message_display(void);
void pass_message_display(uint8_t const * const data, uint8_t const len);
void process_message_display();

#endif /* __display_h__ */