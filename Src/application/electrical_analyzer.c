#include "application/electrical_analyzer.h"

#include "stm32f1xx_hal.h"

#define NUM_CHANNELS     4
#define ADC_BIT_TO_mV(x) (((3300000 / 4095) * x) / 1000)

#define ADC_CHANNEL_LUX         0
#define ADC_CHANNEL_TEMPERATURE 1
#define ADC_CHANNEL_VOLTAGE     2
#define ADC_CHANNEL_CURRENT     3

#define TEMPERATURE_SLOPE     0.025062823967831f
#define TEMPERATURE_INTERCEPT -25.505305176557748f
#define LUX_SLOPE             0.911390660003446f
#define LUX_INTERCEPT         -425.5767706358779f

#define SQUARE_ROOT_x1000 1414

#define VOLTAGE_REDUCED_MAX_mV    3160
#define VOLTAGE_REDUCED_MIN_mV    200
#define VOLTAGE_REAL_MAX_x1000    (127 * SQUARE_ROOT_x1000)
#define VOLTAGE_REDUCED_OFFSET_mV ((VOLTAGE_REDUCED_MIN_mV + VOLTAGE_REDUCED_MAX_mV) / 2)
#define VOLTAGE_REAL_GAIN_x1000                                                          \
    (VOLTAGE_REAL_MAX_x1000 / (VOLTAGE_REDUCED_MAX_mV - VOLTAGE_REDUCED_OFFSET_mV))
#define VOLTAGE_REDUCED_TO_REAL(x) ((VOLTAGE_REAL_GAIN_x1000 * x) / 1000)

#define CURRENT_REDUCED_MAX_mV    2760
#define CURRENT_REDUCED_MIN_mV    80
#define CURRENT_REAL_MAX_x1000    (200 * 1000)
#define CURRENT_REDUCED_OFFSET_mV ((CURRENT_REDUCED_MIN_mV + CURRENT_REDUCED_MAX_mV) / 2)
#define CURRENT_REAL_GAIN_x1000                                                          \
    (CURRENT_REAL_MAX_x1000 / (CURRENT_REDUCED_MAX_mV - CURRENT_REDUCED_OFFSET_mV))
#define CURRENT_REDUCED_TO_REAL(x) ((CURRENT_REAL_GAIN_x1000 * x) / 1000)

extern ADC_HandleTypeDef hadc1;

static uint16_t adc_buf[NUM_CHANNELS];

void init_electrical_analyzer(void) {
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, NUM_CHANNELS);
}

void adc_conversion_complete_callback(void) {}

int16_t get_voltage(void) {
    const int32_t voltage_mV =
        ADC_BIT_TO_mV(adc_buf[ADC_CHANNEL_VOLTAGE]) - VOLTAGE_REDUCED_OFFSET_mV;
    const int16_t voltage_V = (int16_t)VOLTAGE_REDUCED_TO_REAL(voltage_mV);
    return voltage_V;
}

int16_t get_current(void) {
    const int32_t current_mV =
        ADC_BIT_TO_mV(adc_buf[ADC_CHANNEL_CURRENT]) - CURRENT_REDUCED_OFFSET_mV;
    const int16_t current_V = (int16_t)CURRENT_REDUCED_TO_REAL(current_mV);
    return current_V;
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
    return get_voltage() * get_current();
}
