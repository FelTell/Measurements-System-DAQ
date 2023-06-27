/**
 * @file timer_handler.h
 * @author Felipe Telles (felipe.telles@42we.tech)
 * @brief This file allows multiple timers to be implemented in the code
 * @date 22-06-2023
 *
 *
 */

#include "stm32f1xx.h"
#include "stm32f1xx_hal_tim.h"

#include <application/timer_handler.h>

extern TIM_HandleTypeDef htim2;

/**
 * @brief Waits a specific timer to be elapsed by a desired amount of milliseconds. Allows
 * multiple timers by sending a different timer_start.
 *
 * @param timer_start Timer specifier.
 * @param delay Amount desired to wait, max is 2^32, which is equal to
 * approximately 50 days. Values higher than that will not work as expected!
 * @return true The desired time has elapsed.
 * @return false The desired time has not been elapsed.
 */
bool timer_wait_ms(uint32_t timer_start, uint32_t delay) {
    // HAL_GetTick returns the amount of milliseconds that has passed since the chip has
    // been turned on
    const uint32_t current_time = HAL_GetTick();
    if ((current_time - timer_start) >= delay) {
        return true;
    }
    return false;
}

/**
 * @brief Starts to count ticks in microseconds by enabling a new timer peripheral with
 * tick configured to exactly 1 us
 *
 */
void timer_us_init() {
    HAL_TIM_Base_Start(&htim2);
}

/**
 * @brief Updates a timer with the current tick in milliseconds
 *
 * @return uint32_t time in milliseconds that has passed since the chip has been powered
 * up
 */
uint32_t timer_update_ms(void) {
    return __HAL_TIM_GET_COUNTER(&htim2);
}
/**
 * @brief Waits a specific timer to be elapsed by a desired amount of microseconds. Allows
 * multiple timers by sending a different timer_start.
 *
 * @param timer_start Timer specifier.
 * @param delay Amount desired to wait, max is 2^32, which is equal to
 * approximately 1 hour. Values higher than that will not work as expected!
 * @return true The desired time has elapsed.
 * @return false The desired time has not been elapsed.
 */
bool timer_wait_us(uint32_t timer_start, uint32_t delay) {
    // HAL_GetTick returns the amount of milliseconds that has passed since the chip has
    // been turned on
    const uint32_t current_time = __HAL_TIM_GET_COUNTER(&htim2);
    if ((current_time - timer_start) >= delay) {
        return true;
    }
    return false;
}

/**
 * @brief Updates a timer with the current tick in microseconds
 *
 * @return uint32_t time in microseconds that has passed since the chip has been powered
 * up
 */
uint32_t timer_update_us(void) {
    return HAL_GetTick();
}
