#pragma once


#include <stdint.h>

void init_electrical_analyzer(void);

void adc_conversion_complete_callback(void);

int16_t get_lux(void);

int16_t get_temperature(void);

int16_t get_voltage(void);

int16_t get_current(void);

int16_t get_power(void);
