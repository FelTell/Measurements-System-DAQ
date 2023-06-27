/**
 * @file timer_handler.h
 * @author Felipe Telles (felipe.telles@42we.tech)
 * @brief This file allows multiple timers to be implemented in the code
 * @date 22-06-2023
 *
 *
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Waits a specific timer to be elapsed by a desired amount. Allows
 * multiple timers by sending a different timer_start.
 *
 * @param timer_start Timer specifier.
 * @param delay Amount desired to wait, max is 2^32, which is equal to
 * approximately 50 days. Values higher than that will not work as expected!
 * @return true The desired time has elapsed.
 * @return false The desired time has not been elapsed.
 */
bool timer_wait_ms(uint32_t timer_start, uint32_t delay);

/**
 * @brief Updates a timer with the current tick in milliseconds
 *
 * @return uint32_t time in milliseconds that has passed since the chip has been powered
 * up
 */
uint32_t timer_update_ms(void);

/**
 * @brief Starts to count ticks in microseconds by enabling a new timer peripheral with
 * tick configured to exactly 1 us
 *
 */
void timer_us_init();

/**
 * @brief Waits a specific timer to be elapsed by a desired amount of microseconds.
 * Allows multiple timers by sending a different timer_start.
 *
 * @param timer_start Timer specifier.
 * @param delay Amount desired to wait, max is 2^32, which is equal to
 * approximately 1 hour. Values higher than that will not work as expected!
 * @return true The desired time has elapsed.
 * @return false The desired time has not been elapsed.
 */
bool timer_wait_us(uint32_t timer_start, uint32_t delay);

/**
 * @brief Updates a timer with the current tick in microseconds
 *
 * @return uint32_t time in microseconds that has passed since the chip has been
 * powered up
 */
uint32_t timer_update_us(void);
