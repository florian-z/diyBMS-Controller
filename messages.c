#include "messages.h"

/** Decode / Encode bytes */
uint8_t decode_nibble(const uint8_t nibble_char) {
  return nibble_char < 'A' ? nibble_char - '0' : nibble_char + 10 - 'A';
}
uint8_t parse_chars_to_byte(const uint8_t* bytes) {
  uint8_t result = decode_nibble(bytes[0]) * 16;
  result += decode_nibble(bytes[1]);
  return result;
}
uint16_t parse_chars_to_word(const uint8_t* bytes) {
  uint16_t result = decode_nibble(bytes[0]) * 4096;
  result += decode_nibble(bytes[1]) * 256;
  result += decode_nibble(bytes[2]) * 16;
  result += decode_nibble(bytes[3]);
  return result;
}
uint8_t encode_nibble(const uint8_t nibble_value) {
  return nibble_value < 10 ? nibble_value + '0' : nibble_value - 10 + 'A';
}
void format_byte_to_chars(uint8_t* bytes, const uint8_t data) {
  bytes[0] = encode_nibble(data/16);
  bytes[1] = encode_nibble(data%16);
}
void format_word_to_chars(uint8_t* bytes, const uint16_t data) {
  bytes[0] = encode_nibble(data/4096);
  bytes[1] = encode_nibble((data%4096)/256);
  bytes[2] = encode_nibble((data%256)/16);
  bytes[3] = encode_nibble(data%16);
}

void format_dword_to_chars(uint8_t* bytes, const uint32_t data) {
  bytes[0] = encode_nibble(data/268435456);
  bytes[1] = encode_nibble((data%268435456)/16777216);
  bytes[2] = encode_nibble((data%16777216)/1048576);
  bytes[3] = encode_nibble((data%1048576)/65536);
  bytes[4] = encode_nibble((data%65536)/4096);
  bytes[5] = encode_nibble((data%4096)/256);
  bytes[6] = encode_nibble((data%256)/16);
  bytes[7] = encode_nibble(data%16);
}

/** NMEA CRC tools */
void append_nmea_crc(uint8_t * const msg)
{
    const uint8_t MSG_LEN = strlen((char*)msg);
    msg[MSG_LEN] = MSG_CRC;
    uint8_t crc = 0;
    nmea_calc_checksum(msg, &crc);
    format_byte_to_chars(&msg[MSG_LEN+1], crc);
}

bool is_nmea_checksum_good(uint8_t const * const msg)
{
    uint8_t crc_from_msg, crc_calc;

    int8_t retval = nmea_calc_checksum(msg, &crc_calc);

    const uint8_t MSG_LEN = strlen((char*)msg);
    crc_from_msg = parse_chars_to_byte(&msg[MSG_LEN-3]);

    if (retval != MSG_OK || crc_from_msg != crc_calc)
    {
        return false; /* crc bad */
    }
    else
    {
        return true; /* crc good */
    }
}

int8_t nmea_calc_checksum(uint8_t const * const msg, uint8_t * const crc_extern)
{
    uint8_t i = 0, crc = 0;
    *crc_extern = 0xFF;
    /* skip all before '!' or '$' */
    while(msg[i] != MSG_START &&  msg[i] != '$')
    {
        i++;
        if(msg[i] == '\0')
        {
            return MSG_ERR;
        }
    }
    i++;
    /* calc crc until '*' */
    while(msg[i] != MSG_CRC)
    {
        if(msg[i] == '\0')
        {
            /* found unexpected msg-end, msg missing '*' */
            crc = 0xFF;
            return MSG_ERR;
        }

        crc ^= msg[i];
        i++;
    }
    /* no error -> give crc */
    *crc_extern = crc;
    return MSG_OK;
}