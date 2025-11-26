/**
  ******************************************************************************
  * @file           : rotary_encoder.c
  * @brief          : Rotary Encoder driver implementation using TIM2 Encoder Mode
  ******************************************************************************
  */

#include "rotary_encoder.h"
#include "stm32wbxx_hal_def.h"

#include <limits.h>
#include <stdint.h>

/* External timer handle configured in encoder mode */
extern TIM_HandleTypeDef ROTARY_ENCODER_TIMER_HANDLER;

/* Cached counter state */
static uint32_t last_counter_value;
static uint64_t counter_range;
static int32_t pending_ticks;

static void RotaryEncoder_UpdateRange(void)
{
    counter_range = (uint64_t)__HAL_TIM_GET_AUTORELOAD(&ROTARY_ENCODER_TIMER_HANDLER) + 1ULL;
    if (counter_range == 0ULL)
    {
        counter_range = UINT64_C(0x100000000);
    }
}

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
HAL_StatusTypeDef RotaryEncoder_Init(void)
{
    /* Start the encoder timer and capture its counter range */
    if (HAL_TIM_Encoder_Start(&ROTARY_ENCODER_TIMER_HANDLER, TIM_CHANNEL_ALL) != HAL_OK)
    {
        return HAL_ERROR;
    }

    RotaryEncoder_UpdateRange();
    last_counter_value = __HAL_TIM_GET_COUNTER(&ROTARY_ENCODER_TIMER_HANDLER);
    pending_ticks = 0;

    return HAL_OK;
}

/**
 * @brief Get the rotation delta since last call
 * 
 * This function reads the current counter value from TIM2 which is configured
 * in encoder mode. The timer automatically counts up/down based on the quadrature
 * input from the encoder, providing natural sign handling.
 * 
 * @return Delta value - the change in counter position since last call
 *         Positive values indicate clockwise rotation
 *         Negative values indicate counter-clockwise rotation
 *         0 = no rotation since last call
 */
int32_t RotaryEncoder_GetDelta(void)
{
    /* Read the current counter value from TIM2 */
    uint32_t current_count = __HAL_TIM_GET_COUNTER(&ROTARY_ENCODER_TIMER_HANDLER);
    if (counter_range == 0ULL)
    {
        RotaryEncoder_UpdateRange();
    }

    int64_t raw_delta = (int64_t)current_count - (int64_t)last_counter_value;
    int64_t half_range = (int64_t)(counter_range / 2ULL);

    if (raw_delta > half_range)
    {
        raw_delta -= (int64_t)counter_range;
    }
    else if (raw_delta < -half_range)
    {
        raw_delta += (int64_t)counter_range;
    }

    if (raw_delta > INT32_MAX)
    {
        raw_delta = INT32_MAX;
    }
    else if (raw_delta < INT32_MIN)
    {
        raw_delta = INT32_MIN;
    }

    int32_t delta = (int32_t)raw_delta;
    pending_ticks += delta;
    last_counter_value = current_count;

    int32_t logical_delta = pending_ticks / 2;
    if (logical_delta != 0)
    {
        pending_ticks -= logical_delta * 2;
        return logical_delta;
    }

    return 0;
}
