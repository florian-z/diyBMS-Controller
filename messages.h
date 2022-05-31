#ifndef __messages_h__
#define __messages_h__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MSG_OK      0
#define MSG_ERR     -1

#define MSG_START '!'
#define MSG_CRC '*'
#define MSG_END '\n'

/** Decode / Encode bytes */
uint8_t decode_nibble(const uint8_t nibble_char);
uint8_t parse_chars_to_byte(const uint8_t* bytes);
uint16_t parse_chars_to_word(const uint8_t* bytes);
uint32_t parse_chars_to_dword(const uint8_t* bytes);

uint8_t encode_nibble(const uint8_t nibble_value);
void format_byte_to_chars(uint8_t* bytes, const uint8_t data);
void format_word_to_chars(uint8_t* bytes, const uint16_t data);
void format_dword_to_chars(uint8_t* bytes, const uint32_t data);

/** NMEA CRC tools */
void append_nmea_crc(uint8_t * const msg);
bool is_nmea_checksum_good(uint8_t const * const msg);
int8_t nmea_calc_checksum(uint8_t const * const msg, uint8_t * const crc_extern);

#endif /* __messages_h__ */