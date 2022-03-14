#ifndef __messages_h__
#define __messages_h__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MSG_OK      0
#define MSG_ERR     -1

void append_crc(uint8_t * const msg);
bool is_nmea_checksum_good(uint8_t const * const msg);
int8_t nmea_calc_checksum(uint8_t const * const msg, uint8_t * const crc_extern);

#endif /* __display_h__ */