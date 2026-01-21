/**
 ******************************************************************************
 * @file           :  utils.h
 * @brief          :  Utility functions for temperature conversion and scheduling
 *
 * @details        :  Provides helper functions for converting between
 *                    temperature index values and Celsius temperatures,
 *                    generating UI temperature selector strings, and loading
 *                    default heating/cooling schedule presets for 3, 4, or 5
 *                    time slots per day.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#ifndef CORE_INC_UTILS_H
#define CORE_INC_UTILS_H

#include <stddef.h>
#include <stdint.h>

#include "storage_task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Load default schedule into the provided structure
 *
 * @details  Initializes a daily heating schedule with factory preset time
 *           slots and temperatures. Supports three schedule presets:
 *           - 3 slots: 00:00-05:30 (18°C), 05:30-22:00 (20°C), 22:00-00:00 (18°C)
 *           - 4 slots: 00:00-05:30 (18°C), 05:30-15:00 (20°C), 15:00-22:00 (19°C),
 *             22:00-00:00 (18°C)
 *           - 5 slots: 00:00-05:30 (18°C), 05:30-07:00 (20°C), 07:00-15:00 (18°C),
 *             15:00-22:00 (20°C), 22:00-00:00 (18°C)
 *
 * @param  schedule   Pointer to schedule structure to initialize
 * @param  num_slots  Number of time slots to configure (3, 4, or 5)
 *
 * @return void
 *
 * @note   Other slot configurations default to undefined; only specified
 *         num_slots are configured by this function.
 *
 * @see    DailyScheduleTypeDef
 */
void Utils_LoadDefaultSchedule(DailyScheduleTypeDef *schedule,
                               uint8_t num_slots);

/**
 * @brief  Convert temperature index to Celsius float value
 *
 * @details  Converts a 52-value temperature index (0-51) to its Celsius
 *           equivalent. Index uses 0.5°C resolution between 5.0°C and 29.5°C,
 *           with special endpoints for OFF (4.5°C) and ON (30.0°C) states.
 *           Linear interpolation for non-special indices:
 *           - Index 0 → OFF (4.5°C)
 *           - Index 1 → 5.0°C
 *           - Index 2 → 5.5°C
 *           - ...
 *           - Index 50 → 29.5°C
 *           - Index 51 → ON (30.0°C)
 *
 * @param  index  The temperature index value (0-51)
 *
 * @return Temperature value in degrees Celsius (4.5 to 30.0)
 *
 * @note   Index values outside 0-51 are clamped to boundaries
 *
 * @see    Utils_TempToIndex
 */
float Utils_IndexToTemp(uint16_t index);

/**
 * @brief  Convert Celsius temperature value to index
 *
 * @details  Reverse operation of Utils_IndexToTemp(). Converts a Celsius
 *           temperature to its corresponding 52-value index. Clamping applied
 *           for out-of-range temperatures: values ≤4.5°C map to index 0 (OFF),
 *           values ≥30.0°C map to index 51 (ON). Values between 5.0-29.5°C
 *           use 0.5°C resolution.
 *
 * @param  temp  The temperature in degrees Celsius
 *
 * @return Temperature index value (0-51)
 *
 * @note   Floating-point temperatures are converted to nearest index;
 *         intermediate values round toward nearest index
 *
 * @see    Utils_IndexToTemp
 */
uint16_t Utils_TempToIndex(float temp);

/**
 * @brief  Generate temperature option string for UI roller/selector
 *
 * @details  Generates a newline-separated string of all 52 temperature options
 *           for display in a roller widget or scrolling list selector.
 *           Format: "OFF\n5.0\n5.5\n6.0\n...\n29.5\nON" with each temperature
 *           shown with single decimal place precision.
 *
 * @param  buffer  Output buffer to write the options string
 * @param  size    Size of the output buffer (should be ≥120 bytes for full list)
 *
 * @return void
 *
 * @note   Buffer overrun protection should be ensured by caller;
 *         no bounds checking is performed internally
 *
 * @see    Utils_IndexToTemp, Utils_TempToIndex
 */
void Utils_GenerateTempOptions(char *buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_UTILS_H */
