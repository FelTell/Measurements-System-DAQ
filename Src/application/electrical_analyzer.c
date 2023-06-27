#include "application/electrical_analyzer.h"

#include "application/timer_handler.h"
#include "stm32f1xx_hal.h"

#include <math.h>

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

#define PHASE_DELAY_US                 600
#define MINUMUM_SAMPLES_FOR_DATA_READY 10000

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

static uint16_t adc_buf[NUM_CHANNELS];

static uint32_t voltage_sum_of_square;
static uint32_t current_sum_of_square;
static int32_t samples;
static bool data_ready;

static int16_t voltage_rms;
static int16_t current_rms;
static bool is_rms_acquisition_activated = false;

/**
 * @brief Initialize functions needed for the electrical analyzer such as ADC calibration
 * and DMA start
 *
 */
void electrical_analyzer_init(void) {
    // All STM32F1 devices allow self calibration. It should be done after every power-up
    HAL_ADCEx_Calibration_Start(&hadc1);
    // Start to directly transfer ADC results to memory. This means that everytime the ADC
    // conversion is done in every channel the adc_buf vector is updated automatically
    // with no time lost by the ARM core.
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, NUM_CHANNELS);
}

/**
 * @brief This function is like loop() function. It only runs if rms acquisition is
 * activated and there is data ready.
 *
 */
void electrical_analyzer_handler(void) {
    if (!is_rms_acquisition_activated) {
        return;
    }
    if (!data_ready) {
        return;
    }

    voltage_rms = sqrt(voltage_sum_of_square / samples);
    current_rms = sqrt(current_sum_of_square / samples);
    data_ready  = false;
}

/**
 * @brief Allows other files to set the if rms acquisition is activated
 *
 * @param status
 */
void set_is_rms_acquisition_activated(bool status) {
    is_rms_acquisition_activated = status;
}

/**
 * @brief Get the voltage from the value in ADC converted to voltage
 *
 * @return int32_t Voltage in V
 */
int32_t get_instant_voltage(void) {
    return VOLTAGE_BIT_TO_REAL_V(adc_buf[ADC_CHANNEL_VOLTAGE]);
}

/**
 * @brief Get the instant current from the value in ADC converted to voltage
 *
 * @return int32_t current in mA
 */
int32_t get_instant_current(void) {
    return CURRENT_BIT_TO_REAL_mA(adc_buf[ADC_CHANNEL_CURRENT]);
}

/**
 * @brief Get the lux value from the ADC converted to lux level
 *
 * @return float value in lx
 */
float get_lux(void) {
    const uint16_t lux_value = adc_buf[ADC_CHANNEL_LUX];
    if (lux_value < 470) {
        return 0;
    }
    return ((float)(adc_buf[ADC_CHANNEL_LUX]) * LUX_SLOPE) + LUX_INTERCEPT;
}

/**
 * @brief Get the temperature value from the ADC converted to temperature
 *
 * @return float value in celsius
 */
float get_temperature(void) {
    return ((float)(adc_buf[ADC_CHANNEL_TEMPERATURE]) * TEMPERATURE_SLOPE)
           + TEMPERATURE_INTERCEPT;
}

/**
 * @brief Get the instant power value from the voltage and current value multiplied. Since
 * there is a phase lag because of the transformer we wait some time to allow to get the
 * data from both signals at the same moment
 *
 * @return int32_t power in mW
 */
int32_t get_instant_power(void) {
    const int16_t first_value = get_instant_current();
    const uint32_t delay_us   = timer_update_us();
    while (!timer_wait_us(delay_us, PHASE_DELAY_US)) {
        ;
    }
    const int16_t second_value = get_instant_voltage();
    return first_value * second_value;
}

/**
 * @brief Get the voltage rms
 *
 * @return int32_t voltage in V RMS
 */
int32_t get_voltage_rms(void) {
    return voltage_rms;
}

/**
 * @brief Get the current rms
 *
 * @return int32_t current in mA RMS
 */
int32_t get_current_rms(void) {
    return current_rms;
}

/**
 * @brief Get the power rms
 *
 * @return int32_t power in mW RMS
 */
int32_t get_power_rms(void) {
    return voltage_rms * current_rms;
}

/**
 * @brief This callback is called every time there is a completed ADC conversion in all
 * channels. It sums the voltage and current squared value obtained by converting the ADC
 * value. After a defined number of samples has been acquired we consider the data ready
 * to be evaluated. This logic will only run when rms acquisition is activated and no
 * data is ready.
 *
 * @param hadc adc instance
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    static int32_t tmp_current_sum_of_square;
    static int32_t tmp_voltage_sum_of_square;
    static int32_t tmp_samples;

    if (hadc != &hadc1) {
        return;
    }

    if (!is_rms_acquisition_activated) {
        return;
    }

    if (data_ready) {
        return;
    }

    const uint32_t current_value_mA = get_instant_current();
    tmp_current_sum_of_square += current_value_mA * current_value_mA;
    const uint32_t voltage_value_mV = get_instant_voltage();
    tmp_voltage_sum_of_square += voltage_value_mV * voltage_value_mV;
    tmp_samples++;

    if (tmp_samples > MINUMUM_SAMPLES_FOR_DATA_READY) {
        data_ready                = true;
        current_sum_of_square     = tmp_current_sum_of_square;
        voltage_sum_of_square     = tmp_voltage_sum_of_square;
        samples                   = tmp_samples;
        tmp_current_sum_of_square = 0;
        tmp_voltage_sum_of_square = 0;
        tmp_samples               = 0;
    }
}
