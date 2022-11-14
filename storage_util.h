#ifndef __storage_util_h__
#define __storage_util_h__

#include <stdint.h>

#define STORAGE_UTIL_LEN		16384

void store_msg(uint8_t const * const data);

uint8_t* give_buffer_start(void);
uint8_t* give_history_start(void);

/* gives first char of currently selected ble-message. if != '\0' then can read message */
uint8_t has_ble_message(void);
/* give ble-read-pointer to next message (terminated by '\n') */
uint8_t* give_ble_start_and_increment(uint8_t max_len, uint8_t* used_len);

#endif /* __storage_util_h__ */