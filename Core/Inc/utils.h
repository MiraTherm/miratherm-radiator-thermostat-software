#ifndef CORE_INC_UTILS_H
#define CORE_INC_UTILS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert temperature index to float value
 * Index mapping:
 * - Index 0 = OFF (4.5°C)
 * - Index 1-50 = 5.0°C to 29.5°C (0.5°C steps)
 * - Index 51 = ON (30.0°C)
 *
 * @param index The index value (0-51)
 * @return Temperature value in Celsius
 */
float Utils_IndexToTemp(uint16_t index);

/**
 * @brief Convert temperature float value to index
 * Reverse of IndexToTemp function.
 *
 * @param temp The temperature in Celsius
 * @return Index value (0-51)
 */
uint16_t Utils_TempToIndex(float temp);

/**
 * @brief Generate temperature option string for UI roller/selector
 * Generates "OFF\n5.0\n5.5\n...\n29.5\nON" string.
 *
 * @param buffer Output buffer to write the options string
 * @param size Size of the buffer
 */
void Utils_GenerateTempOptions(char *buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_UTILS_H */
