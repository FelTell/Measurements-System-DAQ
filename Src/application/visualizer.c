#include "application/visualizer.h"

#include "application/electrical_analyzer.h"
#include "application/timer_handler.h"
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

static uint32_t timer;

/**
 * @brief updates the frequency of the data visualization
 *
 * @param value new frequency, limited by MIN_FREQUENCY and MAX_FREQUENCY;
 */
void visualizer_update_frequency(int32_t value) {
    char string_to_send[MAX_TX_SIZE];
    int32_t tam;

    if (value >= MIN_FREQUENCY && value <= MAX_FREQUENCY) {
        configured_period_ms = 1000 / value;

        tam = sprintf(string_to_send, "Frequency set as %d Hz, period is %d ms.\n",
                      1000 / configured_period_ms, configured_period_ms);

    } else {
        tam = sprintf(string_to_send,
                      "Value not allowed, allowed frequencies are %d to %d Hz.\n",
                      MIN_FREQUENCY, MAX_FREQUENCY);
    }

    if (tam > MAX_TX_SIZE) {
        return;
    }
    CDC_Transmit_FS((uint8_t*)string_to_send, tam);
}

/**
 * @brief Print the selected channel in the selected fequency
 *
 */
void visualizer_handler(void) {
    if (!timer_wait_ms(timer, configured_period_ms)) {
        return;
    }
    timer = timer_update_ms();

    char string_to_send[MAX_TX_SIZE];
    uint8_t index = 0;

    switch (channel_to_visualize) {
        case channel_none: return;
        case channel_temperature:
            index += sprintf(string_to_send + index, "%.2f °C\t", get_temperature());

            break;
        case channel_lux:
            index += sprintf(string_to_send + index, "%.1f lx\t", get_lux());

            break;
        case channel_voltage:
            index += sprintf(string_to_send + index, "%i V\t", get_instant_voltage());
            break;
        case channel_current:
            index += sprintf(string_to_send + index, "%i mA\t", get_instant_current());
            break;
        case channel_power:
            index += sprintf(string_to_send + index, "%i mW\t", get_instant_power());
            break;
        case channel_voltage_current_power:
            index += sprintf(string_to_send + index, "%i V\t", get_instant_voltage());
            index += sprintf(string_to_send + index, "%i mA\t", get_instant_current());
            index += sprintf(string_to_send + index, "%i mW\t", get_instant_power());
            break;
        case channel_lux_temperature:
            index += sprintf(string_to_send + index, "%.2f °C, \t", get_temperature());
            index += sprintf(string_to_send + index, "%.1f lx\t", get_lux());
            break;
        case channel_voltage_rms:
            index += sprintf(string_to_send + index, "%i Vrms\t", get_voltage_rms());
            break;
        case channel_current_rms:
            index += sprintf(string_to_send + index, "%i Arms\t", get_current_rms());
            break;
        case channel_power_rms:
            index += sprintf(string_to_send + index, "%i mW\t", get_power_rms());
            break;
        case channel_voltage_current_power_rms:
            index += sprintf(string_to_send + index, "%i Vrms, \t", get_voltage_rms());
            index += sprintf(string_to_send + index, "%i mArms, \t", get_current_rms());
            index += sprintf(string_to_send + index, "%i mW\t", get_power_rms());
            break;
        default: {
        }
    }

    sprintf(string_to_send + index++, "\n");

    if (index > MAX_TX_SIZE) {
        return;
    }
    CDC_Transmit_FS((uint8_t*)string_to_send, index);
}

/**
 * @brief Updates the channel which will be printed. Also configures the frequency as the
 * best for the channel
 *
 * @param channel
 */
void visualizer_update_channels(uint8_t channel) {
    char string_to_send[MAX_TX_SIZE];
    int32_t index      = 0;
    uint16_t frequency = 1;

    if (channel >= channel_size) {
        index += sprintf(string_to_send, "Channel not allowed. ");
        channel_to_visualize = 0;
    } else {
        channel_to_visualize = channel;
    }
    index += sprintf(string_to_send + index, "Showing channel: %d", channel_to_visualize);

    switch (channel_to_visualize) {

        case channel_none:
            frequency = 1;
            set_is_rms_acquisition_activated(false);
            break;
        case channel_temperature:
        case channel_lux:
        case channel_lux_temperature:
            frequency = 2;
            set_is_rms_acquisition_activated(false);
            break;
        case channel_voltage:
        case channel_current:
        case channel_power:
        case channel_voltage_current_power:
            frequency = 500;
            set_is_rms_acquisition_activated(false);
            break;
        case channel_voltage_rms:
        case channel_current_rms:
        case channel_power_rms:
        case channel_voltage_current_power_rms:
            frequency = 2;
            set_is_rms_acquisition_activated(true);
            break;
        default: {
        }
    }

    sprintf(string_to_send + index++, "\n");

    CDC_Transmit_FS((uint8_t*)string_to_send, index);
    visualizer_update_frequency(frequency);

    // block the code for 1 second to allow the command to be read
    HAL_Delay(1000);
}
