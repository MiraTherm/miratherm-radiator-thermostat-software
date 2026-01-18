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

void Utils_LoadDefaultSchedule(DailyScheduleTypeDef *schedule,
                               uint8_t num_slots) {
  schedule->NumTimeSlots = num_slots;

  /* Common: Slot 0 is 00:00 - 05:30, 18C */
  schedule->TimeSlots[0].StartHour = 0;
  schedule->TimeSlots[0].StartMinute = 0;
  schedule->TimeSlots[0].EndHour = 5;
  schedule->TimeSlots[0].EndMinute = 30;
  schedule->TimeSlots[0].Temperature = 18.0f;

  if (num_slots == 3) {
    /* Slot 1: 05:30 - 22:00, 20C */
    schedule->TimeSlots[1].StartHour = 5;
    schedule->TimeSlots[1].StartMinute = 30;
    schedule->TimeSlots[1].EndHour = 22;
    schedule->TimeSlots[1].EndMinute = 0;
    schedule->TimeSlots[1].Temperature = 20.0f;

    /* Slot 2: 22:00 - 23:59, 18C */
    schedule->TimeSlots[2].StartHour = 22;
    schedule->TimeSlots[2].StartMinute = 0;
    schedule->TimeSlots[2].EndHour = 23;
    schedule->TimeSlots[2].EndMinute = 59;
    schedule->TimeSlots[2].Temperature = 18.0f;
  } else if (num_slots == 4) {
    /* Slot 1: 05:30 - 15:00, 20C */
    schedule->TimeSlots[1].StartHour = 5;
    schedule->TimeSlots[1].StartMinute = 30;
    schedule->TimeSlots[1].EndHour = 15;
    schedule->TimeSlots[1].EndMinute = 0;
    schedule->TimeSlots[1].Temperature = 20.0f;

    /* Slot 2: 15:00 - 22:00, 19C */
    schedule->TimeSlots[2].StartHour = 15;
    schedule->TimeSlots[2].StartMinute = 0;
    schedule->TimeSlots[2].EndHour = 22;
    schedule->TimeSlots[2].EndMinute = 0;
    schedule->TimeSlots[2].Temperature = 19.0f;

    /* Slot 3: 22:00 - 23:59, 18C */
    schedule->TimeSlots[3].StartHour = 22;
    schedule->TimeSlots[3].StartMinute = 0;
    schedule->TimeSlots[3].EndHour = 23;
    schedule->TimeSlots[3].EndMinute = 59;
    schedule->TimeSlots[3].Temperature = 18.0f;
  } else if (num_slots == 5) {
    /* Slot 1: 05:30 - 07:00, 20C */
    schedule->TimeSlots[1].StartHour = 5;
    schedule->TimeSlots[1].StartMinute = 30;
    schedule->TimeSlots[1].EndHour = 7;
    schedule->TimeSlots[1].EndMinute = 0;
    schedule->TimeSlots[1].Temperature = 20.0f;

    /* Slot 2: 07:00 - 15:00, 18C */
    schedule->TimeSlots[2].StartHour = 7;
    schedule->TimeSlots[2].StartMinute = 0;
    schedule->TimeSlots[2].EndHour = 15;
    schedule->TimeSlots[2].EndMinute = 0;
    schedule->TimeSlots[2].Temperature = 18.0f;

    /* Slot 3: 15:00 - 22:00, 20C */
    schedule->TimeSlots[3].StartHour = 15;
    schedule->TimeSlots[3].StartMinute = 0;
    schedule->TimeSlots[3].EndHour = 22;
    schedule->TimeSlots[3].EndMinute = 0;
    schedule->TimeSlots[3].Temperature = 20.0f;

    /* Slot 4: 22:00 - 23:59, 18C */
    schedule->TimeSlots[4].StartHour = 22;
    schedule->TimeSlots[4].StartMinute = 0;
    schedule->TimeSlots[4].EndHour = 23;
    schedule->TimeSlots[4].EndMinute = 59;
    schedule->TimeSlots[4].Temperature = 18.0f;
  }
}
