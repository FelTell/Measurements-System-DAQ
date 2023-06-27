#include "application/controller.h"

#include "application/electrical_analyzer.h"
#include "application/timer_handler.h"
#include "application/visualizer.h"
#include "main.h"

#include <stdlib.h>
#include <string.h>

#define MAX_RX_SIZE 15

/**
 * @brief Starts data acquisition and turn led on
 *
 */
static void module_start(void);

/**
 * @brief Stops data acquisition and turn leds off
 *
 */
static void module_stop(void);

static bool controller_status = false;

/**
 * @brief Function to be called at the device initialization, similar to a arduino setup()
 *  function.
 *
 */
void controller_init(void) {
    module_stop();
    electrical_analyzer_init();
    timer_us_init();
}

/**
 * @brief Function to be called at code execution, similar to a arduino loop() function.
 * Does nothing if controller status is false
 *
 */
void controller_handler(void) {
    if (!controller_status) {
        return;
    }
    electrical_analyzer_handler();
    visualizer_handler();
}

/**
 * @brief Function which is called when a new message is sent by the USB device
 *
 * @param message Pointer to the received text
 * @param size Size of the received message
 */
void controller_receive_message(char* message, uint32_t size) {
    if (size > MAX_RX_SIZE) {
        return;
    }

    if (strncmp(message, "start", 5) == 0) {
        module_start();
    } else if (strncmp(message, "stop", 4) == 0) {
        module_stop();
    } else if (strncmp(message, "show", 4) == 0) {
        visualizer_update_channels(atoi(&message[4]));
    } else if (strncmp(message, "freq", 4) == 0) {
        visualizer_update_frequency(atoi(&message[4]));
    }
}

static void module_start(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
    controller_status = true;
}

static void module_stop(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
    controller_status = false;
}
