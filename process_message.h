#ifndef __process_message_h__
#define __process_message_h__

#include "main.h"
#include "r_cg_userdefine.h"

void pass_message_usb(uint8_t const * const data, uint8_t const len);
void pass_message_cellmodule(uint8_t const * const data, uint8_t const len, uint8_t const chain_no);
void pass_message_display(uint8_t const * const data, uint8_t const len);

void process_message_usb();
void process_message_cell_module();
void process_message_display();

#endif /* __process_message_h__ */