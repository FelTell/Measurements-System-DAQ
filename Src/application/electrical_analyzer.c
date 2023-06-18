#include "application/electrical_analyzer.h"

#include "stm32f1xx_hal.h"

#define NUM_CHANNELS 4
#define GAIN_ADC     (4.095 / 3.3)

#define ADC_CHANNEL_LUX         0
#define ADC_CHANNEL_TEMPERATURE 1
#define ADC_CHANNEL_VOLTAGE     2
#define ADC_CHANNEL_CURRENT     3

#define TEMPERATURE_SLOPE     0.025062823967831f
#define TEMPERATURE_INTERCEPT -25.505305176557748f
#define LUX_SLOPE             0.911390660003446f
#define LUX_INTERCEPT         -425.5767706358779f

extern ADC_HandleTypeDef hadc1;

static uint16_t adc_buf[NUM_CHANNELS];

void init_electrical_analyzer(void) {
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, NUM_CHANNELS);
}

void adc_conversion_complete_callback(void) {}

int16_t get_voltage(void) {
    return adc_buf[ADC_CHANNEL_VOLTAGE];
}

int16_t get_current(void) {
    return adc_buf[ADC_CHANNEL_CURRENT];
}

float get_lux(void) {
    const uint16_t luxValue = adc_buf[ADC_CHANNEL_LUX];
    if (luxValue < 470) {
        return 0;
    }
    return ((float)(adc_buf[ADC_CHANNEL_LUX]) * LUX_SLOPE) + LUX_INTERCEPT;
}

float get_temperature(void) {
    return ((float)(adc_buf[ADC_CHANNEL_TEMPERATURE]) * TEMPERATURE_SLOPE)
           + TEMPERATURE_INTERCEPT;
}

int16_t get_power(void) {
    return 0;
}
