#ifndef __storage_util_h__
#define __storage_util_h__

#include <stdint.h>

#define STORAGE_UTIL_LEN		16384

void store_msg(uint8_t const * const data);

uint8_t* give_buffer_start(void);
uint8_t* give_history_start(void);

#endif /* __storage_util_h__ */