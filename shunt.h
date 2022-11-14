#ifndef __shunt_h__
#define __shunt_h__

#include "main.h"
#include "r_cg_userdefine.h"
#include <stdbool.h>

#define SHUNT_LEN 10

typedef struct {
    float vshunt;   // mV
    float vbus;     // V
    float dietemp;  // °C
    float current;  // A
    float power;    // W
    float energy;   // Wh
    float charge;   // Ah
} shunt_t;

void shunt_init();
void shunt_tick();

bool shunt_report_charge_start();

void log_shunt_full_debug();
void freezeframe_shunt_full_debug();

void pass_message_shunt();
void process_message_shunt();

#endif /* __shunt_h__ */