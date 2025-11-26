/**
  ******************************************************************************
  * @file           : rotary_encoder.c
  * @brief          : Rotary Encoder driver implementation using LPTIM1
  ******************************************************************************
  */

#include "rotary_encoder.h"

/* External LPTIM1 handle from main.c */
extern LPTIM_HandleTypeDef hlptim1;

/* Static variable to store the last counter value */
static uint32_t last_counter_value = 0;

/**
 * @brief Initialize the Rotary Encoder using LPTIM1
 * 
 * The LPTIM1 must be configured in main.c before calling this function:
 * - Counter source: External
 * - Input1Source: GPIO (connected to RE_A on PB5)
 * - Input2Source: GPIO (connected to RE_B on PB7)
 * - Clock source: APBCLOCK_LPOSC or INTERNAL
 * 
 * @retval HAL_OK on success, HAL_ERROR otherwise
 */
HAL_StatusTypeDef RotaryEncoder_Init(void)
{
    /* Initialize the last counter value */
    last_counter_value = 0;

    /* Start the LPTIM counter with maximum period (0xFFFF) */
    if (HAL_LPTIM_Counter_Start(&hlptim1, 0xFFFFU) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief Get the rotation delta since last call
 * 
 * This function reads the current counter value from LPTIM1, which increments
 * with each edge detected on the encoder inputs. The delta is calculated as
 * the difference between current and last read values.
 * 
 * The LPTIM1 is configured for external counting with two inputs (quadrature).
 * 
 * @return Delta value - the change in counter position since last call
 *         Positive values indicate clockwise rotation
 *         Negative values indicate counter-clockwise rotation
 *         0 = no rotation since last call
 */
int16_t RotaryEncoder_GetDelta(void)
{
    /* Read the current counter value */
    uint32_t current_count = HAL_LPTIM_ReadCounter(&hlptim1);
    
    /* Calculate delta (difference from last read) */
    int16_t delta = (int16_t)(current_count - last_counter_value);
    
    /* Update last counter value */
    last_counter_value = current_count;

    return delta;
}
