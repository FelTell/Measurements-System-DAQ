#pragma once
#include <stdbool.h>
#include <stdint.h>

void visualizer_update_frequency(int32_t requestedFrequency);
void visualizer_update_channels(uint8_t channel);
void visualizer_print_channels(void);
uint16_t visualizer_get_period(void);
