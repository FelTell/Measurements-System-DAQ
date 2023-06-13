#include "application/ntc_data.h"
#include "stm32f1xx_hal.h"
#include "usbd_cdc_if.h"

#include <application/visualizer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHANNEL   3
#define MAX_FREQUENCY 200
#define MIN_FREQUENCY 1
#define MAX_TX_SIZE   100
#define GAIN_ADC      (4.095 / 3.3)

static uint16_t get_lux();
static uint16_t get_voltage();
static uint16_t get_current();
static uint16_t get_power();
static int16_t get_temperature();

enum {
    channel_none,
    channel_temperature,
    channel_lux,
    channel_voltage,
    channel_current,
    channel_power,
    channel_electricity,
    channel_all,
    channel_size
} channel_to_visualize = channel_none;

static uint16_t configured_period_ms = 1000;
static uint16_t adc_buf[MAX_CHANNEL + 1];

extern ADC_HandleTypeDef hadc1;

void visualizer_init(void) {
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, MAX_CHANNEL + 1);
}

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
        case channel_none: break;
        case channel_temperature:
            index += sprintf(stringToSend + index, "%d\t", get_temperature());
            break;
        case channel_lux:
            index += sprintf(stringToSend + index, "%d\t", get_lux());
            break;
        case channel_voltage:
            index += sprintf(stringToSend + index, "%d\t", get_voltage());
            break;
        case channel_current:
            index += sprintf(stringToSend + index, "%d\t", get_current());
            break;
        case channel_power:
            index += sprintf(stringToSend + index, "%d\t", get_power());
            break;
        case channel_electricity:
            index += sprintf(stringToSend + index, "%d\t", get_voltage());
            index += sprintf(stringToSend + index, "%d\t", get_current());
            index += sprintf(stringToSend + index, "%d\t", get_power());
            break;
        case channel_all:
            index += sprintf(stringToSend + index, "%d\t", get_temperature());
            index += sprintf(stringToSend + index, "%d\t", get_lux());
            index += sprintf(stringToSend + index, "%d\t", get_voltage());
            index += sprintf(stringToSend + index, "%d\t", get_current());
            index += sprintf(stringToSend + index, "%d\t", get_power());
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

    sprintf(stringToSend + index++, "\n");

    CDC_Transmit_FS((uint8_t*)stringToSend, index);
}

static int16_t get_temperature() {
    return adc_to_celsius[adc_buf[0]];
}

// TODO (Felipe): do
static uint16_t get_lux() {
    return adc_buf[1];
}

static uint16_t get_voltage() {
    return adc_buf[2];
}

static uint16_t get_current() {
    return adc_buf[3];
}

static uint16_t get_power() {
    return get_voltage() * get_current();
}
