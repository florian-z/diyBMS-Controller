#include "shunt.h"

#include <string.h>
#include "log_util.h"
#include "messages.h"

#define ADDR_READ  1
#define ADDR_WRITE 0

// config
#define ADDR_CONFIG         (0x00<<2)
// select 40mV full scale ADC_RANGE
#define REG_CONFIG_ADCRANGE (1<<4)
#define ADDR_ADC_CONFIG     (0x01<<2)
#define REG_ADC_CONFIG_MODE_CONTINUOUS_SHUNT_AND_VBUS (0xB<<12)
#define REG_ADC_CONFIG_MODE_CONTINUOUS_ALL (0xF<<12)
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

static shunt_t shunt_data = {0};


uint16_t rx_data[SHUNT_LEN] = {0};
uint16_t tx_data[SHUNT_LEN] = {0};
static volatile bool ready_for_transmit = false;
static volatile bool ready_for_process = false;

void shunt_init()
{
    // CS on shunt-IC always on (nCS always low)
    ready_for_transmit = true;

    // set CONFIG
    {
        // select 40mV shunt full scale ADC_RANGE
        uint16_t tx_data[3] = { ADDR_CONFIG | ADDR_WRITE, 0, REG_CONFIG_ADCRANGE};
        uint16_t rx_data[3] = {0};
        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
        log_va("shunt CONFIG wr %02X %02X\n", tx_data[1], tx_data[2]);
    }
    {
        uint16_t tx_data[3] = { ADDR_CONFIG | ADDR_READ};
        uint16_t rx_data[3] = {0};
        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
        log_va("shunt CONFIG rd %02X %02X\n", rx_data[1], rx_data[2]);
    }
    // set ADC_CONFIG
//    {
//        uint16_t tx_data[3] = { ADDR_ADC_CONFIG | ADDR_WRITE, REG_ADC_CONFIG_MODE_CONTINUOUS_ALL, 0}; untested
//        uint16_t rx_data[3] = {0};
//        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
//    }
//    {
//        uint16_t tx_data[3] = { ADDR_ADC_CONFIG | ADDR_READ, 0 ,0};
//        uint16_t rx_data[3] = {0};
//        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
//        log_va("shunt ADC_CONFIG %02X %02X\n", rx_data[1], rx_data[2]);
//    }
    // set SHUNT_CAL
    {
        uint16_t tx_data[3] = { ADDR_SHUNT_CAL | ADDR_WRITE, SHUNT_VAL/256, SHUNT_VAL&0xff};
        uint16_t rx_data[3] = {0};
        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
        log_va("shunt SHUNT_CAL wr %02X %02X\n", tx_data[1], tx_data[2]);
    }
    {
        uint16_t tx_data[3] = { ADDR_SHUNT_CAL | ADDR_READ, 0, 0};
        uint16_t rx_data[3] = {0};
        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
        log_va("shunt SHUNT_CAL rd %02X %02X\n", rx_data[1], rx_data[2]);
    }
}

float read_vshunt();
float read_vbus();
float read_dietemp();
float read_current();
float read_power();
float read_energy();
float read_charge();
void shunt_tick()
{
    static uint8_t round_robin_reader = 0;
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
    }

    memset(tx_data, 0x00, SHUNT_LEN);
    memset(rx_data, 0x00, SHUNT_LEN);

    switch(round_robin_reader)
    {
        case 0:
            read_dietemp();
            break;
        case 17:
            round_robin_reader = UINT8_MAX; // restart round robin
        case 1:
        case 3:
        case 5:
        case 7:
        case 9:
        case 11:
        case 13:
        case 15:
            read_current();
            break;
        case 2:
        case 6:
        case 10:
        case 14:
            read_vbus();
            break;
        case 4:
            read_energy();
            break;
        case 8:
            read_charge();
            break;
        case 12:
            read_power();
            break;
        case 16:
            read_vshunt();
            break;
    }
    round_robin_reader++;
}


void log_shunt()
{
//    float tmp = ((int16_t)(((rx_data[1]&0xff)<<8)|rx_data[2]&0xff))*7.8125e-3;
//    log_va("shunt RESULT TEMP    %04X %04X %04X       %f degC\n", rx_data[0], rx_data[1], rx_data[2], tmp);
}


float read_vshunt()
{
    // returns mV
    // 312.5 nV/LSB when ADCRANGE = 0
    // 78.125 nV/LSB when ADCRANGE = 1
    tx_data[0] = ADDR_VSHUNT | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 4, rx_data);
    //shunt_data.vshunt = ((int32_t)((rx_data[0]<<16 | rx_data[1])>>3))*78.125e-6;
    //log_va("shunt VSHUNT  %04X %04X   %fV\n", rx_data[0], rx_data[1], shunt_data.vshunt);
    return shunt_data.vshunt;
}

float read_vbus()
{
    // returns V
    // 195.3125 μV/LSB
    // 2.13 correction factor for VBUS resistor-devider
    tx_data[0] = ADDR_VBUS | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 4, rx_data);
//    shunt_data.vbus = ((int32_t)((rx_data[0]<<16 | rx_data[1])>>3))*195.3125e-6*2.13;
//    log_va("shunt VBUS    %04X %04X   %fV\n", rx_data[0], rx_data[1], shunt_data.vbus);
    return shunt_data.vbus;
}

float read_dietemp()
{
    // returns °C
    // 7.8125 m°C/LSB
    tx_data[0] = ADDR_DIETEMP | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 3, rx_data);
    //shunt_data.dietemp = ((int16_t)rx_data[0])*7.8125e-3;
    //log_va("shunt TEMP    %04X %04X %04X       %f degC\n", rx_data[0], rx_data[1], rx_data[2], shunt_data.dietemp);
    return shunt_data.dietemp;
}

float read_current()
{
    // returns A
    // CURRENT_LSB
	tx_data[0] = ADDR_CURRENT | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 4, rx_data);
//    shunt_data.current = ((int32_t)((rx_data[0]<<16 | rx_data[1])>>3))*CURRENT_LSB;
//    log_va("shunt CURRENT %04X %04X   %fA\n", rx_data[0], rx_data[1], shunt_data.current);
    return shunt_data.current;
}

float read_power()
{
    // returns W
    // CURRENT_LSB * 3.2
    // 2.13 correction factor for VBUS resistor-devider
	tx_data[0] = ADDR_POWER | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 4, rx_data);
//    shunt_data.power = ((uint32_t)(rx_data[0]<<16 | rx_data[1]))*CURRENT_LSB*3.2*2.13;
//    log_va("shunt POWER   %04X %04X   %fW\n", rx_data[0], rx_data[1], shunt_data.power);
    return shunt_data.power;
}

float read_energy()
{
    // returns Wh
    // CURRENT_LSB * 3.2 * 16
    // 2.13 correction factor for VBUS resistor-devider
	tx_data[0] = ADDR_ENERGY | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 6, rx_data);
//    shunt_data.energy = (((rx_data[0]*65536 + rx_data[1])*65536) + rx_data[2])*CURRENT_LSB*3.2*16*2.13;
//    log_va("shunt ENERGY  %04X %04X %04X  %fWs\n", rx_data[0], rx_data[1], rx_data[2], shunt_data.energy);
    return shunt_data.energy;
}

float read_charge()
{
    // returns Ah
    // CURRENT_LSB
	tx_data[0] = ADDR_CHARGE | ADDR_READ;
	R_Config_RSPI0_Shunt_Send_Receive(tx_data, 6, rx_data);
//    shunt_data.charge = (((rx_data[0]*65536 + rx_data[1])*65536) + rx_data[2])*CURRENT_LSB;
//    log_va("shunt CHARGE  %04X %04X %04X  %fAs\n", rx_data[0], rx_data[1], rx_data[2], shunt_data.charge);
    return shunt_data.charge;
}


void pass_message_shunt()
{
    ready_for_process = true;
}

void process_message_shunt()
{
    if (ready_for_process)
    {
        log_va("SHUNT result tx[0] %02X rx[0] %02X %02X %02X %02X %02X %02X\n", tx_data[0], rx_data[0], rx_data[1], rx_data[2], rx_data[3], rx_data[4], rx_data[5]);
        if (rx_data[0] || !(rx_data[0]|rx_data[1]|rx_data[2]|rx_data[3]|rx_data[4]|rx_data[5]))
        {
            // rx_data[0] should be zero
            // rx_data[1..5] should not all be zero
            // TODO restart ?
            log("SHUNT comm error\n");
        }
        else
        {
            switch(tx_data[0]&~0x03) // ignore lowest two bits (b1 always zero, b0 r/w flag)
            {
                case ADDR_VSHUNT:
                    log("SHUNT recv VSHUNT ");
                    // returns mV
                    // 312.5 nV/LSB when ADCRANGE = 0
                    // 78.125 nV/LSB when ADCRANGE = 1
                    shunt_data.vshunt = ((int32_t)((rx_data[1]&0xff)<<24 | (rx_data[2]&0xff)<<16 | (rx_data[3]&0xff)<<8))*78.125e-6/4096.0;
                    log_va("%.3f mV\n", shunt_data.vshunt);
                    break;
                case ADDR_VBUS:
                    log("SHUNT recv VBUS ");
                    // returns V
                    // 195.3125 μV/LSB
                    // 2.13 correction factor for VBUS resistor-devider
                    shunt_data.vbus = ((int32_t)((rx_data[1]&0xff)<<24 | (rx_data[2]&0xff)<<16 | (rx_data[3]&0xff)<<8))*195.3125e-6*2.13/4096.0;
                    log_va("%.3f V\n", shunt_data.vbus);
                    break;
                case ADDR_DIETEMP:
                    log("SHUNT recv DIETEMP ");
                    // returns °C
                    // 7.8125 m°C/LSB
                    shunt_data.dietemp = ((int16_t)(((rx_data[1]&0xff)<<8)|rx_data[2]&0xff))*7.8125e-3;
                    log_va("%.3f degC\n", shunt_data.dietemp);
                    break;
                case ADDR_CURRENT:
                    // returns A
                    // CURRENT_LSB
                    log("SHUNT recv CURRENT ");
                    shunt_data.current = ((int32_t)((rx_data[1]&0xff)<<24 | (rx_data[2]&0xff)<<16 | (rx_data[3]&0xff)<<8))*CURRENT_LSB/4096.0;
                    log_va("%.3f A\n", shunt_data.current);
                    break;
                case ADDR_POWER:
                    // returns W
                    // CURRENT_LSB * 3.2
                    // 2.13 correction factor for VBUS resistor-devider
                    log("SHUNT recv POWER ");
                    shunt_data.power = ((uint32_t)((rx_data[1]&0xff)<<16 | (rx_data[2]&0xff)<<8 | (rx_data[3]&0xff)))*CURRENT_LSB*3.2*2.13;
                    log_va("%.3f W\n", shunt_data.power);
                    break;
                case ADDR_ENERGY:
                    // returns Wh
                    // CURRENT_LSB * 3.2 * 16
                    // 2.13 correction factor for VBUS resistor-devider
                    log("SHUNT recv ENERGY ");
                    shunt_data.energy = ((uint32_t)((rx_data[1]&0xff)<<24 | (rx_data[2]&0xff)<<16 | (rx_data[3]&0xff)<<8 | (rx_data[4]&0xff)))*CURRENT_LSB*3.2*16*2.13/3600.0;
                    // value read has five bytes, lowest byte is ignored here
                    log_va("%.3f Wh\n", shunt_data.energy);
                    break;
                case ADDR_CHARGE:
                    // returns Ah
                    // CURRENT_LSB
                    log("SHUNT recv CHARGE ");
                    shunt_data.charge = ((int32_t)((rx_data[1]&0xff)<<24 | (rx_data[2]&0xff)<<16 | (rx_data[3]&0xff)<<8 | (rx_data[4]&0xff)))*CURRENT_LSB/3600.0;
                    // value read has five bytes, lowest byte is ignored here
                    log_va("%.3f Ah\n", shunt_data.charge);
                    break;
            }
        }
        ready_for_transmit = true;
        ready_for_process = false;
    }
}


void print_shunt_full_debug()
{
    log_va("[SHUNT %.3fmV %.2fV %.1f°C %.2fA %.2fW %.2fWh %.2fAh]\n",
        shunt_data.vshunt, shunt_data.vbus, shunt_data.dietemp,
        shunt_data.current, shunt_data.power, shunt_data.energy, shunt_data.charge);
}
