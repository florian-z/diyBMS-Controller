#include "messages.h"

void append_nmea_crc(uint8_t * const msg)
{
    const uint8_t MSG_LEN = strlen((char*)msg);
    msg[MSG_LEN] = '*';
    uint8_t crc = 0;
    nmea_calc_checksum(msg, &crc);
    format_byte_to_ascii(&msg[MSG_LEN+1], crc);
}

bool is_nmea_checksum_good(uint8_t const * const msg)
{
    uint8_t crc_from_msg, crc_calc;

    int8_t retval = nmea_calc_checksum(msg, &crc_calc);

    const uint8_t MSG_LEN = strlen((char*)msg);
    parse_ascii_to_byte(&msg[MSG_LEN-3], &crc_from_msg);

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
    while(msg[i] != '!' &&  msg[i] != '$')
    {
        i++;
        if(msg[i] == '\0')
        {
            return MSG_ERR;
        }
    }
    i++;
    /* calc crc until '*' */
    while(msg[i] != '*')
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