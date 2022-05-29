#ifndef __log_util_h__
#define __log_util_h__

#include <stdint.h>

void log(uint8_t const * const data);
void log_va(const char * format, ...);

#endif /* __log_util_h__ */