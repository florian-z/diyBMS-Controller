#include "log_util.h"
#include <stdarg.h>
#include <stdio.h>
#include "uart_usb.h"


void log(uint8_t const * const data)
{
    GLOBAL_INT_STORE_AND_DISABLE
    send_message_usb(data);
    GLOBAL_INT_RESTORE
}

void log_va(const char * format, ...)
{
    GLOBAL_INT_STORE_AND_DISABLE
    static char buf[200];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, 200, format, args);
    va_end(args);

    send_message_usb((uint8_t*)buf);
    GLOBAL_INT_RESTORE
}

void log_hex(uint8_t const * const data, uint8_t const len)
{
    GLOBAL_INT_STORE_AND_DISABLE
    static char buf[200];
    snprintf(buf, 200, "%02X 0x", len);
    for(uint8_t i = 0; i<len; i++)
    {
        snprintf(buf+5+i*3, 200-5-i*3, "%02X ", data[i]);
    }
    snprintf(buf+5+len*3, 200-5-len*3, "\n");
    send_message_usb(data);
    GLOBAL_INT_RESTORE
}