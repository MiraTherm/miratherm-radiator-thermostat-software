/**
 ******************************************************************************
 * @file lvgl_port_display.h
 * @brief Declaration of functions to implement a LVGL port for MiraTherm 
 * radiator thermostat.
 *
 * To replace the display driver, ensure that the corresponding display 
 * resolution is greater than or equal to 128x64. Additionaly to functions 
 * declared here, flush_cb(), rounder_cb(), and set_pixel_cb() should be 
 * implemented in lvgl_port_display.c then. Memory buffer sizes should be 
 * adjusted accordingly.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */
#ifndef LVGL_PORT_DISPLAY_H
#define LVGL_PORT_DISPLAY_H

#include "lvgl.h"

/**
 * @brief Stack size for the LVGL rendering task in bytes.
 *
 * Configured to 4KB (4096 bytes) to accommodate LVGL timer handling
 * and rendering operations on the STM32WB55 microcontroller.
 */
#define LVGL_TASK_STACK_SIZE (1024U * 4U)

/**
 * @brief LVGL task entry point for FreeRTOS.
 *
 * This function runs in an infinite loop and handles LVGL timer callbacks
 * and rendering operations. It acquires the LVGL mutex before each rendering
 * cycle to ensure thread-safe access to LVGL objects.
 *
 * @param[in] argument Unused FreeRTOS task argument.
 *
 * @note This task should have adequate stack size (LVGL_TASK_STACK_SIZE).
 * @note The task runs with a 1ms delay between render cycles.
 *
 * @see display_system_init()
 */
void StartLVGLTask(void *argument);

/**
 * @brief Initialize the display system and LVGL library.
 *
 * This function shall initialize:
 * - LVGL rendering mutex for thread safety
 * - Display driver
 * - LVGL core library
 * - Display buffer and driver configuration
 *
 * This function must be called once during system startup before
 * any LVGL operations or the LVGL rendering task begins.
 *
 * @note Not thread-safe. Call only from the initialization context.
 * @see StartLVGLTask()
 */
void display_system_init(void);

/**
 * @brief Acquire the LVGL rendering mutex.
 *
 * Acquires the mutex protecting LVGL operations. This function blocks
 * indefinitely until the mutex is available. Used internally by the
 * LVGL rendering task to ensure thread-safe access.
 *
 * @return true if the mutex was successfully acquired.
 * @return false if the mutex is not initialized or acquisition failed.
 *
 * @note Must be paired with a call to lv_port_unlock().
 * @see lv_port_unlock()
 */
bool lv_port_lock(void);

/**
 * @brief Release the LVGL rendering mutex.
 *
 * Releases the mutex protecting LVGL operations, allowing other threads
 * to acquire it. Must be called after lv_port_lock() to maintain
 * thread safety.
 *
 * @note This function should be called immediately after LVGL rendering
 *       operations are complete.
 * @see lv_port_lock()
 */
void lv_port_unlock(void);

#endif /* LVGL_PORT_DISPLAY_H */
