#pragma once

#include <stdint.h>

void electrical_analyzer_init(void);

void electrical_analyzer_handler(void);

float get_lux(void);

float get_temperature(void);

int32_t get_voltage(void);

int32_t get_current(void);

int32_t get_power(void);

int32_t get_voltage_rms(void);

int32_t get_current_rms(void);

int32_t get_power_rms(void);
