#ifndef __freeze_util_h__
#define __freeze_util_h__

#include <stdint.h>

void freeze(uint8_t const * const data);
void freeze_va(const char * format, ...);
void freeze_va_no_ts(const char * format, ...);
void freeze_hex(uint8_t const * const data, uint8_t const len);

#endif /* __freeze_util_h__ */