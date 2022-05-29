#include "pcf8574_pwr.h"
#include "string.h"
#include "log_util.h"

#define PCA8574_ADR 0x20    /* 7-bit address without read/write-flag */

/*** PCA8574 I2C PWR ***/
/* PCA7584 all outputs are active low */
#define PCA8574_ALL_OUTPUTS_INACTIVE 0xFF
static volatile uint8_t pca8574_outputs = PCA8574_ALL_OUTPUTS_INACTIVE;

/* buffer commands */
void config_message_pwr(uint8_t const flags)
{
    GLOBAL_INT_STORE_AND_DISABLE
    /* OFF flags have prio over ON flags */
    if(PCA8574_P1_HEAT_OFF & flags) {
        pca8574_outputs &= ~PCA8574_P1_HEAT_OFF;
    } else if (PCA8574_P0_HEAT_ON & flags) {
        pca8574_outputs &= ~PCA8574_P0_HEAT_ON;
    }
    if(PCA8574_P3_BAL_OFF & flags) {
        pca8574_outputs &= ~PCA8574_P3_BAL_OFF;
    } else if (PCA8574_P2_BAL_ON & flags) {
        pca8574_outputs &= ~PCA8574_P2_BAL_ON;
    }
    if(PCA8574_P5_PWR_SHUNT_OFF & flags) {
        pca8574_outputs &= ~PCA8574_P5_PWR_SHUNT_OFF;
    } else if (PCA8574_P4_PWR_SHUNT_ON & flags) {
        pca8574_outputs &= ~PCA8574_P4_PWR_SHUNT_ON;
    }
    if(PCA8574_P7_PWR_MCU_OFF & flags) {
        pca8574_outputs &= ~PCA8574_P7_PWR_MCU_OFF;
    } else if (PCA8574_P6_PWR_MCU_ON & flags) {
        pca8574_outputs &= ~PCA8574_P6_PWR_MCU_ON;
    }
    GLOBAL_INT_RESTORE
}

/* send commands */
void send_message_pwr()
{
    GLOBAL_INT_STORE_AND_DISABLE
    static uint8_t pca8754_output_tx_buf = PCA8574_ALL_OUTPUTS_INACTIVE;
    pca8754_output_tx_buf = pca8574_outputs;
    GLOBAL_INT_RESTORE
    R_Config_RIIC0_PWR_Master_Send(PCA8574_ADR, &pca8754_output_tx_buf, 1);
}

/* callback on transfer success */
void callback_pwr_transfer_success()
{
    GLOBAL_INT_STORE_AND_DISABLE
    pca8574_outputs = PCA8574_ALL_OUTPUTS_INACTIVE; // reset for next transfer
    GLOBAL_INT_RESTORE
    // TODO flo GLOBAL_STATUS_HANDLER(I2C, REPORT_GOOD)
}

/* callback on transfer error */
void callback_pwr_transfer_error()
{
    log("I2C slave - transfer error\n");
    // TODO flo GLOBAL_STATUS_HANDLER(I2C, REPORT_ERROR)
}

/* test - circle all outputs */
//void test_i2c_pwr(void)
//{
//    static uint8_t cnt = 0;
//    cnt++;
//    if(cnt==2) RELAIS_HEAT_ON
//    if(cnt==3) RELAIS_HEAT_OFF
//    if(cnt==5) RELAIS_BALANCER_ON
//    if(cnt==6) RELAIS_BALANCER_OFF
//    if(cnt==8) RELAIS_PWR_MCU_ON
//    if(cnt==9) RELAIS_PWR_MCU_OFF
//    if(cnt==11) RELAIS_PWR_SHUNT_ON
//    if(cnt==12) RELAIS_PWR_SHUNT_OFF
//    if(cnt==20) cnt=0;
//}