#include "shunt.h"

#include <string.h>
#include "log_util.h"
#include "freeze_util.h"
#include "messages.h"

#define ADDR_READ  1
#define ADDR_WRITE 0

// config
#define ADDR_CONFIG         (0x00<<2)
// reset Charge and Energy accumulation registers
#define REG_CONFIG_B0_RSTACC    (0x40)
// select 40mV full scale ADC_RANGE
#define REG_CONFIG_B1_ADCRANGE_40mV (1<<4)

#define ADDR_ADC_CONFIG     (0x01<<2)
#define REG_ADC_CONFIG_B0_MODE_CONTINUOUS_SHUNT_AND_VBUS (0xB<<4)
#define REG_ADC_CONFIG_B0_MODE_CONTINUOUS_ALL (0xF<<4)
#define REG_ADC_CONFIG_B0_VSHUNT_CONV_TIME_4US (0x01)
#define REG_ADC_CONFIG_B1_VSHUNT_CONV_TIME_4US (0xC0)
#define REG_ADC_CONFIG_B1_AVG_4 (0x01)
#define REG_ADC_CONFIG_B1_AVG_16 (0x02)
#define REG_ADC_CONFIG_B1_AVG_64 (0x03)
#define REG_ADC_CONFIG_B1_AVG_128 (0x04)

#define ADDR_SHUNT_CAL      (0x02<<2)
#define ADDR_SHUNT_TEMPCO   (0x03<<2)

// measurements
#define ADDR_VSHUNT         (0x04<<2)   // 24 bit
#define ADDR_VBUS           (0x05<<2)   // 24 bit
#define ADDR_DIETEMP        (0x06<<2)
// calc results
#define ADDR_CURRENT        (0x07<<2)   // 24 bit
#define ADDR_POWER          (0x08<<2)   // 24 bit
#define ADDR_ENERGY         (0x09<<2)   // 40 bit
#define ADDR_CHARGE         (0x0A<<2)   // 40 bit

// thresholds
#define ADDR_DIAG_ALERT     (0x0B<<2)
#define ADDR_SOVL           (0x0C<<2)
#define ADDR_SUVL           (0x0D<<2)
#define ADDR_BOVL           (0x0E<<2)
#define ADDR_BUVL           (0x0F<<2)
#define ADDR_TEMP_LIMIT     (0x10<<2)
#define ADDR_PWR_LIMIT      (0x11<<2)

// ids
#define ADDR_MANUFACTURER_ID (0x3E<<2)
#define ADDR_DEVICE_ID       (0x3F<<2)


// CURRENT_LSB = 262.144A / 2^19 = 0.0005A = 0.5mA
// R_SHUNT = 50mV / 300A = 0.000166667 Ohm
// ADC_RANGE = 1 -> multiply by four
// SHUNT_CAL = 13107.2e6 * CURRENT_LSB * R_SHUNT * 4 = 4369,07
#define SHUNT_VAL 4369
#define CURRENT_LSB 0.0005
// current[A] = CURRENT_LSB * CURRENT
// power[W] = 3.2 * CURRENT_LSB * POWER
// energy[J][Ws] = 16* 3.2 * CURRENT_LSB * ENERGY
// charge[C][As] = CURRENT_LSB * CHARGE
#define VBUS_CORRECTION     1.10254


shunt_t shunt_data = {0};


static uint16_t rx_data[SHUNT_LEN] = {0};
static uint16_t tx_data[SHUNT_LEN] = {0};
static volatile bool ready_for_transmit = false;
static volatile bool ready_for_process = false;

void shunt_init()
{
    ready_for_transmit = true;
}

void read_vshunt();
void read_vbus();
void read_dietemp();
void read_current();
void read_power();
void read_energy();
void read_charge();
void write_config();
void write_config_with_rstacc();
void read_config();
void write_adc_config();
void read_adc_config();
void write_shunt_cal();
void read_shunt_cal();
void read_shunt_manufacturerid();

/* check charge & energy values, reset if negative. returns true, if charge and energy will be reset to zero */
static bool shunt_reset_accu_regs = false;
bool shunt_report_charge_start()
{
    if (shunt_data.charge<0 || shunt_data.energy<0)
    {
        shunt_reset_accu_regs = true;
    }
    return shunt_reset_accu_regs;
}

/* trigger SPI communication */
void shunt_tick()
{
    static uint8_t round_robin_reader = 100;
    static uint8_t inner_round_robin_reader = 0;
    GLOBAL_INT_STORE_AND_DISABLE
    if (!ready_for_transmit)
    {
        GLOBAL_INT_RESTORE
        return;
    }
    else
    {
        ready_for_transmit = false;
        GLOBAL_INT_RESTORE
        LED_BL1_ON
    }

    memset(tx_data, 0x00, SHUNT_LEN);
    memset(rx_data, 0x00, SHUNT_LEN);

    switch(round_robin_reader)
    {
   /// config
        case 100:
            write_config();
            break;
        case 101:
            read_config();
            break;
        case 102:
            write_adc_config();
            break;
        case 103:
            read_adc_config();
            break;
        case 104:
            write_shunt_cal();
            break;
        case 105:
            read_shunt_cal();
            break;
        default:
            round_robin_reader = UINT8_MAX; // restart round robin
            SHUNT_CS_COMMUNICATION_STOP
            ready_for_transmit = true;
            break;

    /// read loop
        case 0:
        case 2:
            read_current();
            break;

        case 1:
            read_vbus();
            break;

        case 3:
            if (shunt_reset_accu_regs)
            {
                shunt_reset_accu_regs = false;
                freezeframe_shunt_full_debug();
                freeze("SHUNT RSTACC\n");
                write_config_with_rstacc();
            }
            else
            {
                switch(inner_round_robin_reader)
                {
                    case 0:
                        read_dietemp();
                        break;
                    case 1:
                        read_energy();
                        break;
                    case 2:
                        read_charge();
                        break;
                    case 3:
                        read_power();
                        break;
                    case 4:
                        read_vshunt();
                        break;
                    case 5:
                        read_shunt_manufacturerid();
                        inner_round_robin_reader = UINT8_MAX; // restart inner round robin
                        break;
                }
                inner_round_robin_reader++;
            }

            round_robin_reader = UINT8_MAX; // restart round robin
            break;
    }
    round_robin_reader++;
}


void read_vshunt()
{
    // returns mV
    // 312.5 nV/LSB when ADCRANGE = 0
    // 78.125 nV/LSB when ADCRANGE = 1
    tx_data[0] = ADDR_VSHUNT | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 4, rx_data);
    //shunt_data.vshunt = ((int32_t)((rx_data[0]<<16 | rx_data[1])>>3))*78.125e-6;
    //log_va("shunt VSHUNT  %04X %04X   %fV\n", rx_data[0], rx_data[1], shunt_data.vshunt);
}

void read_vbus()
{
    // returns V
    // 195.3125 μV/LSB
    // 2.13 correction factor for VBUS resistor-devider
    tx_data[0] = ADDR_VBUS | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 4, rx_data);
//    shunt_data.vbus = ((int32_t)((rx_data[0]<<16 | rx_data[1])>>3))*195.3125e-6*2.13;
//    log_va("shunt VBUS    %04X %04X   %fV\n", rx_data[0], rx_data[1], shunt_data.vbus);
}

void read_dietemp()
{
    // returns °C
    // 7.8125 m°C/LSB
    tx_data[0] = ADDR_DIETEMP | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
    //shunt_data.dietemp = ((int16_t)rx_data[0])*7.8125e-3;
    //log_va("shunt TEMP    %04X %04X %04X       %f degC\n", rx_data[0], rx_data[1], rx_data[2], shunt_data.dietemp);
}

void read_current()
{
    // returns A
    // CURRENT_LSB
	tx_data[0] = ADDR_CURRENT | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 4, rx_data);
//    shunt_data.current = ((int32_t)((rx_data[0]<<16 | rx_data[1])>>3))*CURRENT_LSB;
//    log_va("shunt CURRENT %04X %04X   %fA\n", rx_data[0], rx_data[1], shunt_data.current);
}

void read_power()
{
    // returns W
    // CURRENT_LSB * 3.2
    // 2.13 correction factor for VBUS resistor-devider
	tx_data[0] = ADDR_POWER | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 4, rx_data);
//    shunt_data.power = ((uint32_t)(rx_data[0]<<16 | rx_data[1]))*CURRENT_LSB*3.2*2.13;
//    log_va("shunt POWER   %04X %04X   %fW\n", rx_data[0], rx_data[1], shunt_data.power);
}

void read_energy()
{
    // returns Wh
    // CURRENT_LSB * 3.2 * 16
    // 2.13 correction factor for VBUS resistor-devider
	tx_data[0] = ADDR_ENERGY | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 6, rx_data);
//    shunt_data.energy = (((rx_data[0]*65536 + rx_data[1])*65536) + rx_data[2])*CURRENT_LSB*3.2*16*2.13;
//    log_va("shunt ENERGY  %04X %04X %04X  %fWs\n", rx_data[0], rx_data[1], rx_data[2], shunt_data.energy);
}

void read_charge()
{
    // returns Ah
    // CURRENT_LSB
	tx_data[0] = ADDR_CHARGE | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 6, rx_data);
//    shunt_data.charge = (((rx_data[0]*65536 + rx_data[1])*65536) + rx_data[2])*CURRENT_LSB;
//    log_va("shunt CHARGE  %04X %04X %04X  %fAs\n", rx_data[0], rx_data[1], rx_data[2], shunt_data.charge);
}

/* signal received data */
void pass_message_shunt()
{
    ready_for_process = true;
}

/* process received data */
void process_message_shunt()
{
    if (ready_for_process)
    {
//        log_va("SHUNT result tx[0] %02X rx[0] %02X %02X %02X %02X %02X %02X\n", tx_data[0], rx_data[0], rx_data[1], rx_data[2], rx_data[3], rx_data[4], rx_data[5]);
        if (rx_data[0] || !(rx_data[0]|rx_data[1]|rx_data[2]|rx_data[3]|rx_data[4]|rx_data[5]))
        {
            // rx_data[0] should be zero
            // rx_data[1..5] should not all be zero
            log_va("SHUNT possible comm error for tx[0] %02X\n", tx_data[0]);
        }

        switch(tx_data[0]) // ignore lowest two bits (b1 always zero, b0 r/w flag)
        {
            case ADDR_VSHUNT|ADDR_READ:
//                log("SHUNT recv VSHUNT ");
                // returns mV
                // 312.5 nV/LSB when ADCRANGE = 0
                // 78.125 nV/LSB when ADCRANGE = 1
                shunt_data.vshunt = ((int32_t)((rx_data[1]&0xff)<<24 | (rx_data[2]&0xff)<<16 | (rx_data[3]&0xff)<<8))*78.125e-6/4096.0;
//                log_va("%.4f mV\n", shunt_data.vshunt);
                break;
            case ADDR_VBUS|ADDR_READ:
//                log("SHUNT recv VBUS ");
                // returns V
                // 195.3125 μV/LSB
                // *2.13*VBUS_CORRECTION correction factor for VBUS resistor-devider
                shunt_data.vbus = ((int32_t)((rx_data[1]&0xff)<<24 | (rx_data[2]&0xff)<<16 | (rx_data[3]&0xff)<<8))*195.3125e-6*2.13/4096.0*VBUS_CORRECTION;
//                log_va("%.3f V\n", shunt_data.vbus);
                break;
            case ADDR_DIETEMP|ADDR_READ:
//                log("SHUNT recv DIETEMP ");
                // returns °C
                // 7.8125 m°C/LSB
                shunt_data.dietemp = ((int16_t)(((rx_data[1]&0xff)<<8)|rx_data[2]&0xff))*7.8125e-3;
//                log_va("%.3f degC\n", shunt_data.dietemp);
                break;
            case ADDR_CURRENT|ADDR_READ:
//                log("SHUNT recv CURRENT ");
                // returns A
                // CURRENT_LSB
                shunt_data.current = ((int32_t)((rx_data[1]&0xff)<<24 | (rx_data[2]&0xff)<<16 | (rx_data[3]&0xff)<<8))*CURRENT_LSB/4096.0;
//                log_va("%.3f A\n", shunt_data.current);
                //print_shunt_full_debug();
                break;
            case ADDR_POWER|ADDR_READ:
//                log("SHUNT recv POWER ");
                // returns W
                // CURRENT_LSB * 3.2
                // *2.13*VBUS_CORRECTION correction factor for VBUS resistor-devider
                shunt_data.power = ((uint32_t)((rx_data[1]&0xff)<<16 | (rx_data[2]&0xff)<<8 | (rx_data[3]&0xff)))*CURRENT_LSB*3.2*2.13*VBUS_CORRECTION;
//                log_va("%.3f W\n", shunt_data.power);
                break;
            case ADDR_ENERGY|ADDR_READ:
//                log("SHUNT recv ENERGY ");
                // returns Wh
                // CURRENT_LSB * 3.2 * 16
                // *2.13*VBUS_CORRECTION correction factor for VBUS resistor-devider
                // /3600 -> Ws to Wh
                // *256 -> value read has five bytes, lowest byte is ignored here
                shunt_data.energy = ((uint32_t)((rx_data[1]&0xff)<<24 | (rx_data[2]&0xff)<<16 | (rx_data[3]&0xff)<<8 | (rx_data[4]&0xff)))*CURRENT_LSB*3.2*16*2.13/3600.0*VBUS_CORRECTION*256.0;
//                log_va("%.3f Wh\n", shunt_data.energy);
                break;
            case ADDR_CHARGE|ADDR_READ:
//                log("SHUNT recv CHARGE ");
                // returns Ah
                // CURRENT_LSB
                // /3600 -> As to Ah
                // *256 -> value read has five bytes, lowest byte is ignored here
                shunt_data.charge = ((int32_t)((rx_data[1]&0xff)<<24 | (rx_data[2]&0xff)<<16 | (rx_data[3]&0xff)<<8 | (rx_data[4]&0xff)))*CURRENT_LSB/3600.0*256.0;
//                log_va("%.3f Ah\n", shunt_data.charge);
                break;
            case ADDR_ADC_CONFIG|ADDR_READ:
                log_va("shunt ADC_CONFIG rd %02X %02X\n", rx_data[1], rx_data[2]);
                break;
            case ADDR_CONFIG|ADDR_READ:
                log_va("shunt CONFIG rd %02X %02X\n", rx_data[1], rx_data[2]);
                break;
            case ADDR_SHUNT_CAL|ADDR_READ:
                log_va("shunt SHUNT_CAL rd %02X %02X\n", rx_data[1], rx_data[2]);
                break;
            case ADDR_MANUFACTURER_ID|ADDR_READ:
                //log_va("shunt MANUF_ID rd %02X %02X\n", rx_data[1], rx_data[2]);
                if ((rx_data[1] == 0x54) && (rx_data[2] == 0x49))
                {
                    // read value matches expected value -> SPI communication working
                    report_system_status(SHUNT);
                }
                break;
        }
        ready_for_transmit = true;
        ready_for_process = false;
        LED_BL1_OFF
    }
}

/* print all cellmodule data values and ages */
void log_shunt_full_debug()
{
    log_va("[SHUNT %.5fmV %.3fV %.1fC %.3fA %.3fW %.4fWh %.4fAh]\n",
        shunt_data.vshunt, shunt_data.vbus, shunt_data.dietemp,
        shunt_data.current, shunt_data.power, shunt_data.energy, shunt_data.charge);
}
/* store all cellmodule data values and ages */
void freezeframe_shunt_full_debug()
{
    freeze_va("[SHUNT %.4fmV %.2fV %.0fC %.2fA %.2fW %.3fWh %.3fAh]\n",
        shunt_data.vshunt, shunt_data.vbus, shunt_data.dietemp,
        shunt_data.current, shunt_data.power, shunt_data.energy, shunt_data.charge);
}


void write_config()
{
    // set CONFIG
    // select 40.96mV shunt full scale ADC_RANGE
    tx_data[0] = ADDR_CONFIG | ADDR_WRITE;
    tx_data[1] = 0;
    tx_data[2] = REG_CONFIG_B1_ADCRANGE_40mV;
    R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
    log_va("shunt CONFIG wr %02X %02X\n", tx_data[1], tx_data[2]);
}
void write_config_with_rstacc()
{
    // set CONFIG
    // select 40.96mV shunt full scale ADC_RANGE
    // reset accumulations registers Charge and Energy
    tx_data[0] = ADDR_CONFIG | ADDR_WRITE;
    tx_data[1] = REG_CONFIG_B0_RSTACC;
    tx_data[2] = REG_CONFIG_B1_ADCRANGE_40mV;
    R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
    log_va("shunt CONFIG wr %02X %02X\n", tx_data[1], tx_data[2]);
}
void read_config()
{
    // get CONFIG
    tx_data[0] = ADDR_CONFIG | ADDR_READ;
    R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
}
void write_adc_config()
{
    // set ADC_CONFIG
    tx_data[0] = ADDR_ADC_CONFIG | ADDR_WRITE;
    tx_data[1] = REG_ADC_CONFIG_B0_MODE_CONTINUOUS_ALL | REG_ADC_CONFIG_B0_VSHUNT_CONV_TIME_4US;
    tx_data[2] = REG_ADC_CONFIG_B1_VSHUNT_CONV_TIME_4US | REG_ADC_CONFIG_B1_AVG_64;
    R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
    log_va("shunt ADC_CONFIG wr %02X %02X\n", tx_data[1], tx_data[2]);
}
void read_adc_config()
{
    // get ADC_CONFIG
    tx_data[0] = ADDR_ADC_CONFIG | ADDR_READ;
    R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
}
void write_shunt_cal()
{
    // set SHUNT_CAL
    tx_data[0] = ADDR_SHUNT_CAL | ADDR_WRITE;
    tx_data[1] = SHUNT_VAL / 256;
    tx_data[2] = SHUNT_VAL & 0xff;
    R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
    log_va("shunt SHUNT_CAL wr %02X %02X\n", tx_data[1], tx_data[2]);
}
void read_shunt_cal()
{
    // get SHUNT_CAL
    tx_data[0] = ADDR_SHUNT_CAL | ADDR_READ;
    R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
}
void read_shunt_manufacturerid()
{
    tx_data[0] = ADDR_MANUFACTURER_ID | ADDR_READ;
    R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
}
