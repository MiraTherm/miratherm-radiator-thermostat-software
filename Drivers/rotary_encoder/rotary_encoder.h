/**
  ******************************************************************************
  * @file           : rotary_encoder.h
  * @brief          : Rotary Encoder driver header using LPTIM1
  ******************************************************************************
  */

#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32wbxx_hal.h"

/**
 * @brief Initialize the Rotary Encoder
 * Uses LPTIM1 (must be declared as extern in main.c)
 * @retval HAL_OK on success, HAL_ERROR otherwise
 */
HAL_StatusTypeDef RotaryEncoder_Init(void);

/**
 * @brief Get the rotation delta (change in position) since last call
 * Uses LPTIM1 (must be declared as extern in main.c)
 * @return Delta value (positive = clockwise, negative = counter-clockwise)
 */
int16_t RotaryEncoder_GetDelta(void);

#ifdef __cplusplus
}
#endif

#endif /* ROTARY_ENCODER_H */
