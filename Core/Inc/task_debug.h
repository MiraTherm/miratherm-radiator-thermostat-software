/**
 ******************************************************************************
 * @file           :  task_debug.h
 * @brief          :  Task debugging and diagnostic configuration flags
 *
 * @details        :  Provides compile-time configuration switches for enabling
 *                    debug output, error handling diagnostics, and LED
 *                    visualization of system state and task operations. All
 *                    flags default to 0 (disabled) to minimize output and
 *                    performance overhead in production builds.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#ifndef CORE_INC_TASK_DEBUG_H
#define CORE_INC_TASK_DEBUG_H

/**
 * @def OS_TASKS_DEBUG
 *
 * @brief  Enable logging of FreeRTOS task creation and startup sequence
 *
 * @details  When enabled (set to 1), logs detailed information about task
 *           creation, thread handles, stack allocation, and startup transitions
 *           through system initialization. Useful for verifying correct task
 *           creation order and identifying initialization issues. Default: 0
 *           (disabled). Set to 1 to enable debug output via stdout/serial.
 */
#ifndef OS_TASKS_DEBUG
#define OS_TASKS_DEBUG 0
#endif

/**
 * @def ERROR_HANDLER_ON_TASK_CREATION_FAILURE
 *
 * @brief  Enable aggressive error handling and system halt on task creation failures
 *
 * @details  When enabled (set to 1), system halts immediately upon detecting
 *           error conditions on task creation rather than running without the 
 *           failed task. Default: 0 (disabled).
 */
#ifndef ERROR_HANDLER_ON_TASK_CREATION_FAILURE
#define ERROR_HANDLER_ON_TASK_CREATION_FAILURE 0
#endif

/**
 * @def INPUT_TASK_DEBUG_PRINTING
 *
 * @brief  Enable verbose logging of input event processing
 *
 * @details  When enabled (set to 1), logs all button press/release events,
 *           rotary encoder movements, and input queue operations. Helps
 *           diagnose input responsiveness issues and button debounce problems.
 *           Default: 0 (disabled). Set to 1 to debug input handling.
 */
#ifndef INPUT_TASK_DEBUG_PRINTING
#define INPUT_TASK_DEBUG_PRINTING 0
#endif

/**
 * @def SENSOR_TASK_DEBUG_PRINTING
 *
 * @brief  Enable diagnostic logging of sensor measurements and calibration
 *
 * @details  When enabled (set to 1), logs raw ADC conversions, calibrated
 *           sensor values (temperature, battery voltage, motor current),
 *           DMA buffer status, and sensor update timing. Useful for verifying
 *           sensor accuracy and calibration offsets. Default: 0 (disabled).
 *           Set to 1 to troubleshoot sensor issues.
 */
#ifndef SENSOR_TASK_DEBUG_PRINTING
#define SENSOR_TASK_DEBUG_PRINTING 0
#endif

/**
 * @def VIEW_PRESENTER_TASK_DEBUG_PRINTING
 *
 * @brief  Enable logging of UI presentation and state transitions
 *
 * @details  When enabled (set to 1), logs view/presenter lifecycle events,
 *           state machine transitions, display buffer operations, and event
 *           routing. Helps diagnose UI rendering issues and state machine
 *           problems. Default: 0 (disabled). Set to 1 to debug UI behavior.
 */
#ifndef VIEW_PRESENTER_TASK_DEBUG_PRINTING
#define VIEW_PRESENTER_TASK_DEBUG_PRINTING 0
#endif

/**
 * @def VIEW_PRESENTER_TASK_DEBUG_LEDS
 *
 * @brief  Enable LED visualization of system state and task activity
 *
 * @details  When enabled (set to 1), uses RGB LED indicators to display:
 *           - Current system state (initialization, configuration, running, etc.)
 *           - Active task operations (sensor reading, display update, user input)
 *           - Alert conditions (adaptation in progress, errors detected)
 *           - Boost mode activation (temporary temperature override)
 *           Provides real-time visual feedback without serial output.
 *           Default: 0 (disabled). Set to 1 for visual task monitoring.
 */
#ifndef VIEW_PRESENTER_TASK_DEBUG_LEDS
#define VIEW_PRESENTER_TASK_DEBUG_LEDS 0
#endif

#endif /* CORE_INC_TASK_DEBUG_H */
