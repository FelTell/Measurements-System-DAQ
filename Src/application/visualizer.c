#include "application/visualizer.h"

#include "application/electrical_analyzer.h"
#include "usbd_cdc_if.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FREQUENCY 500
#define MIN_FREQUENCY 1
#define MAX_TX_SIZE   100

enum {
    channel_none,
    channel_temperature,
    channel_lux,
    channel_voltage,
    channel_current,
    channel_power,
    channel_voltage_current_power,
    channel_lux_temperature,
    channel_voltage_rms,
    channel_current_rms,
    channel_power_rms,
    channel_voltage_current_power_rms,
    channel_size
} channel_to_visualize = channel_none;

static uint16_t configured_period_ms = 1000;

void visualizer_update_frequency(int32_t value) {
    char stringToSend[MAX_TX_SIZE];
    int32_t tam;

    if (value >= MIN_FREQUENCY && value <= MAX_FREQUENCY) {
        configured_period_ms = 1000 / value;

        tam = sprintf(stringToSend, "Frequency set as %d Hz, period is %d ms.\n",
                      1000 / configured_period_ms, configured_period_ms);

    } else {
        tam = sprintf(stringToSend,
                      "Value not allowed, allowed frequencies are %d to %d Hz.\n",
                      MIN_FREQUENCY, MAX_FREQUENCY);
    }

    if (tam > MAX_TX_SIZE) {
        return;
    }
    CDC_Transmit_FS((uint8_t*)stringToSend, tam);
}

void visualizer_print_channels(void) {
    char stringToSend[MAX_TX_SIZE];
    uint8_t index = 0;

    switch (channel_to_visualize) {
        case channel_none: return;
        case channel_temperature:
            index += sprintf(stringToSend + index, "%.2f °C\t", get_temperature());

            break;
        case channel_lux:
            index += sprintf(stringToSend + index, "%.1f lx\t", get_lux());

            break;
        case channel_voltage:
            index += sprintf(stringToSend + index, "%i V\t", get_voltage());
            break;
        case channel_current:
            index += sprintf(stringToSend + index, "%i mA\t", get_current());
            break;
        case channel_power:
            index += sprintf(stringToSend + index, "%i mW\t", get_power());
            break;
        case channel_lux_temperature:
            index += sprintf(stringToSend + index, "%.2f °C, \t", get_temperature());
            index += sprintf(stringToSend + index, "%.1f lx\t", get_lux());
            break;

        case channel_voltage_rms:
            index += sprintf(stringToSend + index, "%i Vrms\t", get_voltage_rms());
            break;
        case channel_current_rms:
            index += sprintf(stringToSend + index, "%i Arms\t", get_current_rms());
            break;
        case channel_power_rms:
            index += sprintf(stringToSend + index, "%i mW\t", get_power_rms());
            break;
        case channel_voltage_current_power_rms:
            index += sprintf(stringToSend + index, "%i Vrms, \t", get_voltage_rms());
            index += sprintf(stringToSend + index, "%i mArms, \t", get_current_rms());
            index += sprintf(stringToSend + index, "%i mW\t", get_power_rms());
            break;
        default: {
        }
    }

    sprintf(stringToSend + index++, "\n");

    if (index > MAX_TX_SIZE) {
        return;
    }
    CDC_Transmit_FS((uint8_t*)stringToSend, index);
}

uint16_t visualizer_get_period(void) {
    return configured_period_ms;
}

void visualizer_update_channels(uint8_t channel) {
    char stringToSend[MAX_TX_SIZE];
    int32_t index = 0;

    if (channel >= channel_size) {
        index += sprintf(stringToSend, "Channel not allowed. ");
    } else {
        channel_to_visualize = channel;
    }
    index += sprintf(stringToSend + index, "Showing channel: %d", channel_to_visualize);

    switch (channel_to_visualize) {

        case channel_none: visualizer_update_frequency(1); break;
        case channel_temperature:
        case channel_lux:
        case channel_lux_temperature: visualizer_update_frequency(2); break;
        case channel_voltage:
        case channel_current:
        case channel_power:
        case channel_voltage_current_power: visualizer_update_frequency(500); break;
        case channel_voltage_rms:
        case channel_current_rms:
        case channel_power_rms:
        case channel_voltage_current_power_rms: visualizer_update_frequency(2); break;
        default: {
        }
    }

    sprintf(stringToSend + index++, "\n");

    CDC_Transmit_FS((uint8_t*)stringToSend, index);
}
