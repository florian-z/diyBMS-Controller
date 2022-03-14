#ifndef __cellmodule_h__
#define __cellmodule_h__

#include "main.h"
#include "r_cg_userdefine.h"

void send_message_cellmodule(uint8_t const * const data);
void send_message_cellmodule_done(uint8_t const chain_no);
void pass_message_cellmodule(uint8_t const * const data, uint8_t const len, uint8_t const chain_no);
void process_message_cellmodule();

#endif /* __cellmodule_h__ */