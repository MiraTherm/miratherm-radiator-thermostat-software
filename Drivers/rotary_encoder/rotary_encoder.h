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
 * The timer counter is re-centered to 127 after each measurement, so the
 * driver assumes the caller polls fast enough that the count stays within
 * 0..255. The raw delta is batched in pairs (two ticks per reported step)
 * so only odd ticks count (KY-040 issue).
 * @return Delta value (positive = clockwise, negative = counter-clockwise)
 */
int8_t RotaryEncoder_GetDelta(void);

#ifdef __cplusplus
}
#endif

#endif /* ROTARY_ENCODER_H */
