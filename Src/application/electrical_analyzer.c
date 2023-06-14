#include "application/electrical_analyzer.h"

#include "application/ntc_data.h"
#include "stm32f1xx_hal.h"

#define MAX_CHANNEL 3
#define GAIN_ADC    (4.095 / 3.3)

extern ADC_HandleTypeDef hadc1;

static uint16_t adc_buf[MAX_CHANNEL + 1];

void init_electrical_analyzer(void) {
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, MAX_CHANNEL + 1);
}

void adc_conversion_complete_callback(void) {}

int16_t get_voltage(void) {
    return adc_buf[0];
}

int16_t get_current(void) {
    return adc_buf[1];
}

int16_t get_lux(void) {
    return adc_buf[2];
}

int16_t get_temperature(void) {
    return adc_buf[3];
}

int16_t get_power(void) {
    return adc_buf[0];
}
