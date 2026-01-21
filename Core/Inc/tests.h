/**
 ******************************************************************************
 * @file           :  tests.h
 * @brief          :  Test and driver validation framework
 *
 * @details        :  Provides test mode compilation flags and driver test
 *                    interface for hardware validation and component testing.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#ifndef TESTS_H
#define TESTS_H

#include "cmsis_os2.h"
#include "storage_task.h"

/* Forward declaration to break circular dependency with sensor_task.h */
typedef struct SensorValuesAccessTypeDef SensorValuesAccessTypeDef;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def TESTS
 * @brief Master test enable flag
 * @details Set to 1 to enable test mode, 0 for normal operation
 */
#define TESTS 0

/**
 * @def DRIVER_TEST
 * @brief Driver component test mode
 * @details Enables driver test interface for validating hardware drivers
 *          (motor, sensor, buttons, display). Provides interactive UI for
 *          testing individual components.
 */
#define DRIVER_TEST 0

/**
 * @def ADAPTATION_TEST
 * @brief Radiator adaptation algorithm test mode (not implemented yet)
 */
#define ADAPTATION_TEST 0

#if DRIVER_TEST
/**
 * @brief Run driver validation test suite
 * @details Provides interactive UI for testing individual hardware components:
 *          motor control, sensor readings, button input, display output.
 *          Requires DRIVER_TEST flag to be enabled during compilation.
 * @param storage2system_event_queue Event queue from storage task (config load)
 * @param input2vp_event_queue Input event queue (button/encoder events)
 * @param config_access Shared configuration data access
 * @param sensor_values_access Shared sensor measurement values access
 * @return void; infinite loop monitoring test interface
 * @note Called from main task initialization when DRIVER_TEST is enabled
 * @see DRIVER_TEST flag, StartMainTask
 */
void Driver_Test(osMessageQueueId_t storage2system_event_queue,
                 osMessageQueueId_t input2vp_event_queue,
                 ConfigAccessTypeDef *config_access,
                 SensorValuesAccessTypeDef *sensor_values_access);
#elif ADAPTATION_TEST
/**
 * @brief Run radiator adaptation algorithm test (not implemented yet)
 */
void Adaptation_Test(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* TESTS_H */