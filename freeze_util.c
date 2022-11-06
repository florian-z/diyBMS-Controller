#include "freeze_util.h"
#include "log_util.h"
#include <stdarg.h>
#include <stdio.h>
#include "storage_util.h"


void freeze(uint8_t const * const data)
{
    GLOBAL_INT_STORE_AND_DISABLE
    store_msg(data);
    GLOBAL_INT_RESTORE
}

void freeze_va(const char * format, ...)
{
    GLOBAL_INT_STORE_AND_DISABLE
    static char buf[LOG_BUF];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, LOG_BUF, format, args);
    va_end(args);

    store_msg((uint8_t*)buf);
    GLOBAL_INT_RESTORE
}

void freeze_hex(uint8_t const * const data, uint8_t const len)
{
    GLOBAL_INT_STORE_AND_DISABLE
    static char buf[LOG_BUF_HEX];
    snprintf(buf, LOG_BUF_HEX, "%02X 0x", len);
    for(uint8_t i = 0; i<len; i++)
    {
        snprintf(buf+5+i*3, LOG_BUF_HEX-5-i*3, "%02X ", data[i]);
    }
    snprintf(buf+5+len*3, LOG_BUF_HEX-5-len*3, "\n");

    store_msg((uint8_t*)buf);
    GLOBAL_INT_RESTORE
}