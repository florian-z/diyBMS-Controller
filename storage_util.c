#include "storage_util.h"
#include <string.h>
#include <stdlib.h>

#include "log_util.h"

#define STORAGE_LEN		2048

/*** message storage ***/
static uint8_t storage_buf[STORAGE_LEN] = {0};     /* storage buffer */
static uint8_t* storage_buf_wr = storage_buf;      /* write pointer */
static uint8_t* storage_buf_rd = storage_buf;      /* read pointer */

void pointer_rollover(uint8_t* ptr)
{
	if(ptr >= storage_buf + STORAGE_LEN - 1)
	{
		ptr = storage_buf;
	}
}

/* buffer incoming messages */
void store_msg(uint8_t const * const data)
{
    GLOBAL_INT_STORE_AND_DISABLE
    const uint8_t DATA_LEN = strlen((char*)data);
    for(uint8_t i = 0; i<DATA_LEN; i++)
    {
        /* if data gets overwritten */
        if(*storage_buf_wr != '\0')
        {
            /* remove oldest msg */
			uint8_t* temp_wr = storage_buf_wr;
			while(*temp_wr!='\n') // blank everything until msg end
			{
				*temp_wr++ = '\0';
				pointer_rollover(temp_wr);
			}
			*temp_wr++ = '\0'; // remove msg end ('\n')
			pointer_rollover(temp_wr);
			storage_buf_rd = temp_wr; // set read ptr to next msg
        }
        *storage_buf_wr++ = data[i];
        /* write pointer roll over */
		pointer_rollover(storage_buf_wr);
    }
    GLOBAL_INT_RESTORE
}

/* called when ready for next transmission, checks for further pending transmission */
uint8_t* give_msg_start(void)
{
	return storage_buf_rd;
}
