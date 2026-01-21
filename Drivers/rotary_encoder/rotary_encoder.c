/**
 ******************************************************************************
 * @file           :  rotary_encoder.c
 * @brief          :  Implementation of rotary encoder driver using TIM2
 *                    in encoder mode.
 *
 * @details        :  Provides rotary encoder support via STM32 timer in
 *                    quadrature encoder mode. Converts raw timer pulses into
 *                    logical rotation steps, handling KY-040 encoder behavior.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */
#include "rotary_encoder.h"
#include "stm32wbxx_hal_def.h"

#include <stdint.h>

/* External timer handle configured in encoder mode */
extern TIM_HandleTypeDef ROTARY_ENCODER_TIMER_HANDLER;

/* Cached counter state from previous read */
static uint8_t last_counter_value;

/* Accumulator for fractional ticks (two raw ticks = one logical step) */
static int8_t pending_ticks;

/* Center point for 16-bit counter to allow balanced range */
static const uint8_t ENCODER_CENTER = 127U;

/* Initialize rotary encoder: start timer and center counter */
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

/* Read current counter and compute delta, accounting for KY-040 two-tick-per-step behavior */
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
