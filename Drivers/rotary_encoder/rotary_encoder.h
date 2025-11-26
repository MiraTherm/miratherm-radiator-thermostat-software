/**
  ******************************************************************************
  * @file           : rotary_encoder.h
  * @brief          : Rotary Encoder driver header for TIM2 encoder mode
  ******************************************************************************
  */

#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32wbxx_hal.h"

#ifndef ROTARY_ENCODER_TIMER_HANDLER
#define ROTARY_ENCODER_TIMER_HANDLER    htim2
#endif

/**
 * @brief Initialize the Rotary Encoder
 * Uses the timer configured through `ROTARY_ENCODER_TIMER_HANDLER` (defaults to TIM2).
 * The timer must run in encoder mode before calling this function.
 * @retval HAL_OK on success, HAL_ERROR otherwise
 */
HAL_StatusTypeDef RotaryEncoder_Init(void);

/**
 * @brief Get the rotation delta (change in position) since last call
 * The driver accumulates raw ticks and only reports a nonzero logical step when
 * at least two raw ticks have been collected in either direction. Single ticks
 * are buffered until the next call, then they can combine with additional ticks
 * to reach the two-tick threshold.
 * @return Delta value (positive = clockwise, negative = counter-clockwise)
 */
int32_t RotaryEncoder_GetDelta(void);

#ifdef __cplusplus
}
#endif

#endif /* ROTARY_ENCODER_H */
