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

void shunt_init()
{
    // CS on shunt-IC always on (nCS always low)

//    // set CONFIG
//    {
//        // select 40mV shunt full scale ADC_RANGE
//        uint16_t tx_data[2] = { ADDR_CONFIG | ADDR_WRITE, REG_CONFIG_ADCRANGE};
//        uint16_t rx_data[2] = {0};
//        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 2, rx_data);
//    }
//    {
//        uint16_t tx_data[1] = { ADDR_CONFIG | ADDR_READ};
//        uint16_t rx_data[1] = {0};
//        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 1, rx_data);
//        log_va("shunt CONFIG %04X\n", rx_data[0]);
//    }
//    // set ADC_CONFIG
//    {
//        uint16_t tx_data[2] = { ADDR_ADC_CONFIG | ADDR_WRITE, REG_ADC_CONFIG_MODE_CONTINUOUS_ALL};
//        uint16_t rx_data[2] = {0};
//        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 2, rx_data);
//    }
//    {
//        uint16_t tx_data[1] = { ADDR_ADC_CONFIG | ADDR_READ};
//        uint16_t rx_data[1] = {0};
//        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 1, rx_data);
//        log_va("shunt ADC_CONFIG %04X\n", rx_data[0]);
//    }
//    // set SHUNT_CAL
//    {
//        uint16_t tx_data[2] = { ADDR_SHUNT_CAL | ADDR_WRITE, SHUNT_VAL};
//        uint16_t rx_data[2] = {0};
//        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 2, rx_data);
//    }
//    {
//        uint16_t tx_data[1] = { ADDR_SHUNT_CAL | ADDR_READ};
//        uint16_t rx_data[1] = {0};
//        R_Config_RSPI0_Shunt_Send_Receive(tx_data, 1, rx_data);
//        log_va("shunt SHUNT_CAL %04X\n", rx_data[0]);
//    }
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
//    read_vshunt();
//    read_vbus();
    read_dietemp();
//    read_current();
//    read_power();
//    read_energy();
//    read_charge();
}

static uint16_t rx_data[3] = {0};
void log_shunt()
{
    float tmp = ((int16_t)rx_data[0])*7.8125e-3;
    log_va("shunt RESULT TEMP    %04X        %f degC\n", rx_data[0], tmp);
}


float read_vshunt()
{
    // returns mV
    // 312.5 nV/LSB when ADCRANGE = 0
    // 78.125 nV/LSB when ADCRANGE = 1
    uint16_t tx_data = { ADDR_VSHUNT | ADDR_READ };
    uint16_t rx_data[2] = {0};
	R_Config_RSPI0_Shunt_Send_Receive(&tx_data, 1, rx_data);
    shunt_data.vshunt = ((int32_t)((rx_data[0]<<16 | rx_data[1])>>3))*78.125e-6;
    log_va("shunt VSHUNT  %04X %04X   %fV\n", rx_data[0], rx_data[1], shunt_data.vshunt);
    return shunt_data.vshunt;
}

float read_vbus()
{
    // returns V
    // 195.3125 μV/LSB
    // 2.13 correction factor for VBUS resistor-devider
    uint16_t tx_data = { ADDR_VBUS | ADDR_READ };
    uint16_t rx_data[2] = {0};
	R_Config_RSPI0_Shunt_Send_Receive(&tx_data, 1, rx_data);
    shunt_data.vbus = ((int32_t)((rx_data[0]<<16 | rx_data[1])>>3))*195.3125e-6*2.13;
    log_va("shunt VBUS    %04X %04X   %fV\n", rx_data[0], rx_data[1], shunt_data.vbus);
    return shunt_data.vbus;
}

float read_dietemp()
{
    // returns °C
    // 7.8125 m°C/LSB
    uint16_t tx_data = { ADDR_DIETEMP | ADDR_READ };

	R_Config_RSPI0_Shunt_Send_Receive(&tx_data, 3, rx_data);
    shunt_data.dietemp = ((int16_t)rx_data[0])*7.8125e-3;
    log_va("shunt TEMP    %04X        %f degC\n", rx_data[0], shunt_data.dietemp);
    return shunt_data.dietemp;
}

float read_current()
{
    // returns A
    // CURRENT_LSB
	uint16_t tx_data = { ADDR_CURRENT | ADDR_READ };
    uint16_t rx_data[2] = {0};
	R_Config_RSPI0_Shunt_Send_Receive(&tx_data, 1, rx_data);
    shunt_data.current = ((int32_t)((rx_data[0]<<16 | rx_data[1])>>3))*CURRENT_LSB;
    log_va("shunt CURRENT %04X %04X   %fA\n", rx_data[0], rx_data[1], shunt_data.current);
    return shunt_data.current;
}

float read_power()
{
    // returns W
    // CURRENT_LSB * 3.2
    // 2.13 correction factor for VBUS resistor-devider
	uint16_t tx_data = { ADDR_POWER | ADDR_READ };
    uint16_t rx_data[2] = {0};
	R_Config_RSPI0_Shunt_Send_Receive(&tx_data, 1, rx_data);
    shunt_data.power = ((uint32_t)(rx_data[0]<<16 | rx_data[1]))*CURRENT_LSB*3.2*2.13;
    log_va("shunt POWER   %04X %04X   %fW\n", rx_data[0], rx_data[1], shunt_data.power);
    return shunt_data.power;
}

float read_energy()
{
    // returns Wh
    // CURRENT_LSB * 3.2 * 16
    // 2.13 correction factor for VBUS resistor-devider
	uint16_t tx_data = { ADDR_ENERGY | ADDR_READ };
    uint16_t rx_data[3] = {0};
	R_Config_RSPI0_Shunt_Send_Receive(&tx_data, 1, rx_data);
    shunt_data.energy = (((rx_data[0]*65536 + rx_data[1])*65536) + rx_data[2])*CURRENT_LSB*3.2*16*2.13;
    log_va("shunt ENERGY  %04X %04X %04X  %fWs\n", rx_data[0], rx_data[1], rx_data[2], shunt_data.energy);
    return shunt_data.energy;
}

float read_charge()
{
    // returns Ah
    // CURRENT_LSB
	uint16_t tx_data = { ADDR_CHARGE | ADDR_READ };
    uint16_t rx_data[3] = {0};
	R_Config_RSPI0_Shunt_Send_Receive(&tx_data, 1, rx_data);
    shunt_data.charge = (((rx_data[0]*65536 + rx_data[1])*65536) + rx_data[2])*CURRENT_LSB;
    log_va("shunt CHARGE  %04X %04X %04X  %fAs\n", rx_data[0], rx_data[1], rx_data[2], shunt_data.charge);
    return shunt_data.charge;
}

void print_shunt_full_debug()
{
    log_va("[SHUNT %.3fmV %.2fV %.1f°C %.2fA %.2fW %.2fWh %.2fAh]\n",
        shunt_data.vshunt, shunt_data.vbus, shunt_data.dietemp,
        shunt_data.current, shunt_data.power, shunt_data.energy, shunt_data.charge);
}
