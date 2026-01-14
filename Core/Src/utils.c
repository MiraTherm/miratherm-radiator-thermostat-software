#include "utils.h"
#include <stdio.h>
#include <string.h>

float Utils_IndexToTemp(uint16_t index) {
  /* Index 0 = OFF (4.5) */
  /* Index 1 = 5.0 */
  /* ... */
  /* Index 50 = 29.5 */
  /* Index 51 = ON (30.0) */

  if (index == 0)
    return 4.5f;
  if (index == 51)
    return 30.0f;

  return 5.0f + (index - 1) * 0.5f;
}

uint16_t Utils_TempToIndex(float temp) {
  if (temp <= 4.5f)
    return 0;
  if (temp >= 30.0f)
    return 51;

  return (uint16_t)((temp - 5.0f) * 2.0f) + 1;
}

void Utils_GenerateTempOptions(char *buffer, size_t size) {
  /* OFF, 5.0, 5.5 ... 29.5, ON */
  /* OFF = 4.5, ON = 30.0 */

  strcpy(buffer, "OFF\n");
  char temp_str[16];

  for (int i = 10; i < 60; i++) /* 5.0 (10) to 29.5 (59) */
  {
    float t = i / 2.0f;
    snprintf(temp_str, sizeof(temp_str), "%.1f\n", t);
    strcat(buffer, temp_str);
  }
  strcat(buffer, "ON");
  (void)size; /* unused parameter */
}
