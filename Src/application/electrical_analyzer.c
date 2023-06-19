#include "application/electrical_analyzer.h"

#include "application/timer_handler.h"
#include "stm32f1xx_hal.h"

#include <math.h>
#include <stdbool.h>

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

// Defines about voltage acquisition
#define VOLTAGE_REDUCED_MAX_mV 3160
#define VOLTAGE_REDUCED_MIN_mV 200
#define VOLTAGE_REAL_MAX_mV    (127 * SQUARE_ROOT_x1000)

#define VOLTAGE_REDUCED_OFFSET_mV ((VOLTAGE_REDUCED_MIN_mV + VOLTAGE_REDUCED_MAX_mV) / 2)
#define VOLTAGE_GAIN                                                                     \
    (VOLTAGE_REAL_MAX_mV / (VOLTAGE_REDUCED_MAX_mV - VOLTAGE_REDUCED_OFFSET_mV))

#define VOLTAGE_BIT_TO_REDUCED_mV(x) (ADC_BIT_TO_mV(x) - VOLTAGE_REDUCED_OFFSET_mV)
#define VOLTAGE_BIT_TO_REAL_V(x)     (VOLTAGE_GAIN * VOLTAGE_BIT_TO_REDUCED_mV(x) / 1000)

// Defines about current acquisition
#define CURRENT_REDUCED_MAX_mV 2760
#define CURRENT_REDUCED_MIN_mV 80
#define CURRENT_REAL_MAX_uA    (200 * 1000)

#define CURRENT_REDUCED_OFFSET_mV ((CURRENT_REDUCED_MIN_mV + CURRENT_REDUCED_MAX_mV) / 2)
#define CURRENT_GAIN                                                                     \
    (CURRENT_REAL_MAX_uA / (CURRENT_REDUCED_MAX_mV - CURRENT_REDUCED_OFFSET_mV))

#define CURRENT_BIT_TO_REDUCED_mV(x) (ADC_BIT_TO_mV(x) - CURRENT_REDUCED_OFFSET_mV)
#define CURRENT_BIT_TO_REAL_mA(x)    ((CURRENT_GAIN * CURRENT_BIT_TO_REDUCED_mV(x)) / 1000)

// extern HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

static uint16_t adc_buf[NUM_CHANNELS];

static uint32_t timer;

static int32_t voltage_sum_of_square;
static int32_t current_sum_of_square;
static int32_t n;
static bool data_ready;

static int16_t voltage_rms;
static int16_t current_rms;

void electrical_analyzer_init(void) {
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, NUM_CHANNELS);
    timer = timer_update();
}

void electrical_analyzer_handler(void) {
    if (!data_ready) {
        return;
    }
    voltage_rms = sqrt(voltage_sum_of_square / n);
    current_rms = sqrt(current_sum_of_square / n);
    data_ready  = false;
    timer       = timer_update();
}

int32_t get_voltage(void) {
    return VOLTAGE_BIT_TO_REAL_V(adc_buf[ADC_CHANNEL_VOLTAGE]);
}

int32_t get_current(void) {
    return CURRENT_BIT_TO_REAL_mA(adc_buf[ADC_CHANNEL_CURRENT]);
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

int32_t get_power(void) {
    return get_voltage() * get_current();
}

int32_t get_voltage_rms(void) {
    return voltage_rms;
}

int32_t get_current_rms(void) {
    return current_rms;
}

int32_t get_power_rms(void) {
    return voltage_rms * current_rms;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    (void)hadc; // Unused
    static int32_t tmp_current_sum_of_square;
    static int32_t tmp_voltage_sum_of_square;
    static int32_t tmpN;

    if (data_ready) {
        return;
    }

    const int32_t current_value_mA = CURRENT_BIT_TO_REAL_mA(adc_buf[ADC_CHANNEL_CURRENT]);
    tmp_current_sum_of_square += current_value_mA * current_value_mA;
    const int32_t voltage_value_mV = VOLTAGE_BIT_TO_REAL_V(adc_buf[ADC_CHANNEL_VOLTAGE]);
    tmp_voltage_sum_of_square += voltage_value_mV * voltage_value_mV;
    tmpN++;

    if (timer_wait_ms(timer, 250)) {
        data_ready                = true;
        current_sum_of_square     = tmp_current_sum_of_square;
        voltage_sum_of_square     = tmp_voltage_sum_of_square;
        n                         = tmpN;
        tmp_current_sum_of_square = 0;
        tmp_voltage_sum_of_square = 0;
        tmpN                      = 0;
    }
}
