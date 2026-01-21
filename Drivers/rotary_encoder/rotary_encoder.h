/**
 ******************************************************************************
 * @file           :  rotary_encoder.h
 * @brief          :  Declaration of functions for rotary encoder driver using
 *                    TIM2 in encoder mode for MiraTherm radiator thermostat.
 *
 * @details        :  Provides a rotary encoder interface for quadrature input
 *                    on two GPIO pins. Uses STM32 timer in encoder mode to
 *                    count pulses. The driver accumulates raw ticks and reports
 *                    logical steps, accounting for the KY-040 encoder behavior
 *                    which generates two ticks per detent position.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */
#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32wbxx_hal.h"

/**
 * @brief Timer handler macro for the rotary encoder.
 * 
 * Defaults to TIM2. Can be overridden by defining ROTARY_ENCODER_TIMER_HANDLER
 * before including this header. The timer must be configured in encoder mode
 * with quadrature input on channels 1 and 2.
 */
#ifndef ROTARY_ENCODER_TIMER_HANDLER
#define ROTARY_ENCODER_TIMER_HANDLER htim2
#endif

/**
 * @brief Initialize the rotary encoder driver.
 * 
 * Starts the timer configured through ROTARY_ENCODER_TIMER_HANDLER (defaults to TIM2)
 * in encoder mode. The timer must be pre-configured in CubeMX with:
 * - Mode: Encoder Mode (quadrature input from channels 1 and 2)
 * - Channel 1 Input: GPIO connected to encoder phase A
 * - Channel 2 Input: GPIO connected to encoder phase B
 * - Period: 0xFFFF (16-bit counter for full range)
 * 
 * This function centers the counter at 127 to allow equal range for positive
 * and negative rotations (0-127 and 128-255).
 * 
 * @return HAL_OK if initialization successful, HAL_ERROR otherwise.
 * 
 * @note Not thread-safe. Call only from the initialization context.
 * @see RotaryEncoder_GetDelta()
 */
HAL_StatusTypeDef RotaryEncoder_Init(void);

/**
 * @brief Get rotation delta (change in position) since last call.
 * 
 * Reads the current timer counter value and calculates the delta from the
 * previous reading. The counter is re-centered to 127 after each measurement
 * to ensure the counter stays within 0-255 range between calls.
 * 
 * The raw delta is accumulated and reported in pairs (two ticks per logical step)
 * to account for KY-040 rotary encoder behavior which generates two pulses per
 * detent position.
 * 
 * @return Logical delta value:
 *         - Positive values: clockwise rotation (each step = +1)
 *         - Negative values: counter-clockwise rotation (each step = -1)
 *         - 0: no rotation or incomplete steps since last call
 * 
 * @note Must be called frequently enough that counter does not overflow
 *       (assumes less than 128 ticks between calls).
 * @note Thread-safe if timer counter reads are atomic (typical for STM32).
 * @see RotaryEncoder_Init()
 */
int8_t RotaryEncoder_GetDelta(void);

#ifdef __cplusplus
}
#endif

#endif /* ROTARY_ENCODER_H */
