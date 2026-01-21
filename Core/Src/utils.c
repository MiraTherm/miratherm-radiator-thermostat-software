/**
 ******************************************************************************
 * @file           :  utils.c
 * @brief          :  Implementation of utility functions for temperature and scheduling
 *
 * @details        :  Implements temperature index-to-Celsius conversion with
 *                    special OFF/ON endpoints, reverse conversion for UI input,
 *                    temperature option string generation for UI components,
 *                    and factory preset schedule initialization for daily
 *                    heating/cooling time slots.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#include "utils.h"
#include <stdio.h>
#include <string.h>

/**
 * Convert temperature index to Celsius float value.
 * Supports 52 indices: OFF (4.5°C), 0.5°C steps from 5.0°C to 29.5°C, ON (30.0°C).
 */
float Utils_IndexToTemp(uint16_t index) {
  /* Special endpoints: index 0 = OFF (4.5°C), index 51 = ON (30.0°C) */
  if (index == 0)
    return 4.5f;
  if (index == 51)
    return 30.0f;

  /* Linear interpolation: index N → 5.0 + (N-1) × 0.5 */
  return 5.0f + (index - 1) * 0.5f;
}

/**
 * Reverse: convert Celsius temperature to index (0-51).
 * Clamps out-of-range values to endpoints.
 */
uint16_t Utils_TempToIndex(float temp) {
  /* Clamp to special endpoints */
  if (temp <= 4.5f)
    return 0;  /* OFF */
  if (temp >= 30.0f)
    return 51; /* ON */

  /* Reverse interpolation: map 5.0-29.5°C to indices 1-50 using 0.5°C steps */
  return (uint16_t)((temp - 5.0f) * 2.0f) + 1;
}

/**
 * Generate newline-separated temperature option string for UI roller widget.
 * Produces 52 options: OFF, 5.0, 5.5, ..., 29.5, ON
 */
void Utils_GenerateTempOptions(char *buffer, size_t size) {
  /* Start with OFF endpoint */
  strcpy(buffer, "OFF\n");
  char temp_str[16];

  /* Generate 0.5°C steps from 5.0 to 29.5 (indices 1-50) */
  for (int i = 10; i < 60; i++) { /* i=10 → 5.0°C, i=59 → 29.5°C */
    float t = i / 2.0f;
    snprintf(temp_str, sizeof(temp_str), "%.1f\n", t);
    strcat(buffer, temp_str);
  }
  /* End with ON endpoint */
  strcat(buffer, "ON");
  (void)size; /* unused: buffer size validation handled by caller */
}

/**
 * Load factory preset daily heating/cooling schedule.
 * Supports 3-slot, 4-slot, or 5-slot configurations with predefined times/temps.
 */
void Utils_LoadDefaultSchedule(DailyScheduleTypeDef *schedule,
                               uint8_t num_slots) {
  schedule->NumTimeSlots = num_slots;

  /* Slot 0: Common across all presets (00:00-05:30, low night temperature 18°C) */
  schedule->TimeSlots[0].StartHour = 0;
  schedule->TimeSlots[0].StartMinute = 0;
  schedule->TimeSlots[0].EndHour = 5;
  schedule->TimeSlots[0].EndMinute = 30;
  schedule->TimeSlots[0].Temperature = 18.0f;

  /* Load schedule preset based on slot count */
  if (num_slots == 3) {
    /* 3-slot preset: morning comfort (05:30-22:00), night/early morning low */
    schedule->TimeSlots[1].StartHour = 5;
    schedule->TimeSlots[1].StartMinute = 30;
    schedule->TimeSlots[1].EndHour = 22;
    schedule->TimeSlots[1].EndMinute = 0;
    schedule->TimeSlots[1].Temperature = 20.0f; /* Day comfort */

    schedule->TimeSlots[2].StartHour = 22;
    schedule->TimeSlots[2].StartMinute = 0;
    schedule->TimeSlots[2].EndHour = 23;
    schedule->TimeSlots[2].EndMinute = 59;
    schedule->TimeSlots[2].Temperature = 18.0f; /* Evening low */
  } else if (num_slots == 4) {
    /* 4-slot preset: morning high (05:30-15:00), afternoon mid (15:00-22:00), night low */
    schedule->TimeSlots[1].StartHour = 5;
    schedule->TimeSlots[1].StartMinute = 30;
    schedule->TimeSlots[1].EndHour = 15;
    schedule->TimeSlots[1].EndMinute = 0;
    schedule->TimeSlots[1].Temperature = 20.0f; /* Morning comfort */

    schedule->TimeSlots[2].StartHour = 15;
    schedule->TimeSlots[2].StartMinute = 0;
    schedule->TimeSlots[2].EndHour = 22;
    schedule->TimeSlots[2].EndMinute = 0;
    schedule->TimeSlots[2].Temperature = 19.0f; /* Afternoon moderate */

    schedule->TimeSlots[3].StartHour = 22;
    schedule->TimeSlots[3].StartMinute = 0;
    schedule->TimeSlots[3].EndHour = 23;
    schedule->TimeSlots[3].EndMinute = 59;
    schedule->TimeSlots[3].Temperature = 18.0f; /* Evening low */
  } else if (num_slots == 5) {
    /* 5-slot preset: morning peak (05:30-07:00), day low (07:00-15:00), */
    /*               afternoon peak (15:00-22:00), night low (22:00-00:00) */
    schedule->TimeSlots[1].StartHour = 5;
    schedule->TimeSlots[1].StartMinute = 30;
    schedule->TimeSlots[1].EndHour = 7;
    schedule->TimeSlots[1].EndMinute = 0;
    schedule->TimeSlots[1].Temperature = 20.0f; /* Morning peak */

    schedule->TimeSlots[2].StartHour = 7;
    schedule->TimeSlots[2].StartMinute = 0;
    schedule->TimeSlots[2].EndHour = 15;
    schedule->TimeSlots[2].EndMinute = 0;
    schedule->TimeSlots[2].Temperature = 18.0f; /* Day low */

    schedule->TimeSlots[3].StartHour = 15;
    schedule->TimeSlots[3].StartMinute = 0;
    schedule->TimeSlots[3].EndHour = 22;
    schedule->TimeSlots[3].EndMinute = 0;
    schedule->TimeSlots[3].Temperature = 20.0f; /* Afternoon peak */

    schedule->TimeSlots[4].StartHour = 22;
    schedule->TimeSlots[4].StartMinute = 0;
    schedule->TimeSlots[4].EndHour = 23;
    schedule->TimeSlots[4].EndMinute = 59;
    schedule->TimeSlots[4].Temperature = 18.0f; /* Night low */
  }
}
