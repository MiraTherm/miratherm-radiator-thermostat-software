/**
 ******************************************************************************
 * @file           : rotary_encoder.c
 * @brief          : Rotary Encoder driver implementation using TIM2 Encoder
 *Mode
 ******************************************************************************
 */

#include "rotary_encoder.h"
#include "stm32wbxx_hal_def.h"

#include <stdint.h>

/* External timer handle configured in encoder mode */
extern TIM_HandleTypeDef ROTARY_ENCODER_TIMER_HANDLER;

/* Cached counter state */
static uint8_t last_counter_value;
static int8_t pending_ticks;

static const uint8_t ENCODER_CENTER = 127U;

/**
 * @brief Initialize the Rotary Encoder using TIM2 Encoder Mode
 *
 * The TIM2 must be configured in CubeMX before calling this function:
 * - Mode: Encoder Mode (quadrature input from channels 1 and 2)
 * - Input1: GPIO (connected to encoder A)
 * - Input2: GPIO (connected to encoder B)
 * - Period: 0xFFFF (16-bit counter)
 *
 * @retval HAL_OK on success, HAL_ERROR otherwise
 */
HAL_StatusTypeDef RotaryEncoder_Init(void) {
  /* Start the encoder timer and capture its counter range */
  if (HAL_TIM_Encoder_Start(&ROTARY_ENCODER_TIMER_HANDLER, TIM_CHANNEL_ALL) !=
      HAL_OK) {
    return HAL_ERROR;
  }

  __HAL_TIM_SET_COUNTER(&ROTARY_ENCODER_TIMER_HANDLER, ENCODER_CENTER);
  last_counter_value = ENCODER_CENTER;
  pending_ticks = 0;

  return HAL_OK;
}

/**
 * @brief Get the rotation delta since last call
 *
 * This function reads the current counter value from TIM2 which is configured
 * in encoder mode. The timer counter is re-centered to 127 after each read so
 * the driver can assume the current count stays within 0..255 between reads.
 *
 * @return Delta value - the change in counter position since last call
 *         Positive values indicate clockwise rotation
 *         Negative values indicate counter-clockwise rotation
 *         0 = no rotation since last call
 */
int8_t RotaryEncoder_GetDelta(void) {
  /* Read the current counter value from TIM2 */
  uint8_t current_count =
      (uint8_t)__HAL_TIM_GET_COUNTER(&ROTARY_ENCODER_TIMER_HANDLER);
  int8_t delta = (int8_t)((int16_t)current_count - (int16_t)last_counter_value);
  pending_ticks += delta;
  last_counter_value = ENCODER_CENTER;
  __HAL_TIM_SET_COUNTER(&ROTARY_ENCODER_TIMER_HANDLER, ENCODER_CENTER);

  int8_t logical_delta = pending_ticks / 2;
  if (logical_delta != 0) {
    pending_ticks -= logical_delta * 2;
    return logical_delta;
  }

  return 0;
}
