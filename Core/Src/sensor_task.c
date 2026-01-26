/**
 ******************************************************************************
 * @file           :  sensor_task.c
 * @brief          :  Implementation of sensor measurement and ADC data
 *                    acquisition task
 *
 * @details        :  Provides ADC sampling, calibration calculations, battery
 *                    state-of-charge computation, and thread-safe sensor value
 *                    updates via mutex protection.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */
#include "sensor_task.h"

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "main.h"
#include "stm32wbxx_hal.h"
#include "stm32wbxx_hal_adc_ex.h"
#include "stm32wbxx_ll_adc.h"
#include "storage_task.h"
#include "task.h"
#include "task_debug.h"
#include "tests.h"

#include <stddef.h>
#include <string.h>

/* ADC configuration: 4 channels (VREF, Motor, Temperature, VBat) */
#define SENSOR_TASK_ADC_CHANNEL_COUNT 4U
#define SENSOR_TASK_VREF_CHANNEL_INDEX 0U
#define SENSOR_TASK_MOTOR_CHANNEL_INDEX 1U
#define SENSOR_TASK_TEMPERATURE_CHANNEL_INDEX 2U
#define SENSOR_TASK_VBAT_CHANNEL_INDEX 3U

/* Motor shunt resistance and battery divider */
#define SENSOR_TASK_MOTOR_SHUNT_OHMS 0.22f
#define SENSOR_TASK_VBAT_DIVIDER 3.0f

/* DMA buffer for ADC conversions */
static uint16_t s_adc_dma_buffer[SENSOR_TASK_ADC_CHANNEL_COUNT];

/* Thread-safe access to sensor values and configuration */
static SensorValuesAccessTypeDef *s_sensor_values_access = NULL;
static ConfigAccessTypeDef *s_config_access = NULL;

/* Motor measurement enable flag (used to switch measurement periods) */
#if TESTS & DRIVER_TEST
static bool s_motor_measurements_enabled = true;
#else
static bool s_motor_measurements_enabled = false;
#endif

extern ADC_HandleTypeDef hadc1;

/* Convert milliseconds to FreeRTOS ticks, ensuring minimum of 1 tick */
static TickType_t safe_ms_to_ticks(uint32_t ms) {
  const uint32_t safe_ms = (ms == 0U) ? 1U : ms;
  return pdMS_TO_TICKS(safe_ms);
}

/* Calculate internal voltage reference from ADC sample */
static uint32_t calculate_vref_voltage(uint16_t vref_raw) {
  if (vref_raw == 0U) {
    return TEMPSENSOR_CAL_VREFANALOG;
  }
  return __LL_ADC_CALC_VREFANALOG_VOLTAGE(vref_raw, LL_ADC_RESOLUTION_12B);
}

/* Convert ADC raw value to voltage using current VREF calibration */
static float convert_raw_to_voltage(uint16_t raw_value, uint32_t vref_mv) {
  const uint32_t millivolt =
      __LL_ADC_CALC_DATA_TO_VOLTAGE(vref_mv, raw_value, LL_ADC_RESOLUTION_12B);
  return (float)millivolt * 0.001f;
}

static float calculate_temperature(uint16_t temperature_raw, uint32_t vref_mv) {
  const int32_t temperature = __LL_ADC_CALC_TEMPERATURE(
      vref_mv, temperature_raw, LL_ADC_RESOLUTION_12B);

  /* Read temperature offset from config access */
  float offset = 0.0f;
  if (osMutexAcquire(s_config_access->mutex, osWaitForever) == osOK) {
    offset = s_config_access->data.temperature_offset;
    osMutexRelease(s_config_access->mutex);
  }

  return (float)temperature + offset;
}

/* Calculate battery state-of-charge percentage from voltage
   Uses piecewise linear interpolation of 2 AA alkaline discharge curve */
static uint8_t calculate_battery_soc(float battery_voltage_v) {
  typedef struct {
    uint16_t mv;
    uint8_t soc;
  } BatteryPoint;

  /* Discharge curve for 2 AA alkaline batteries (voltage in mV)
     Doubled from single-battery curve due to 2-cell series configuration */
  static const BatteryPoint curve[] = {
      {3200, 100}, /* Freshly out of pack (2 x 1.6V) */
      {3000, 100}, /* Nominal Full (2 x 1.5V) */
      {2800, 85},  /* High (2 x 1.4V) */
      {2600, 60},  /* Mid (2 x 1.3V) */
      {2400, 35},  /* Low (2 x 1.2V) */
      {2200, 10},  /* Critical (2 x 1.1V) */
      {2000, 0}    /* Cutoff (2 x 1.0V) */
  };

  /* Convert from volts to millivolts */
  uint16_t voltage_mv = (uint16_t)(battery_voltage_v * 1000.0f);

  /* Handle boundary cases */
  if (voltage_mv >= curve[0].mv)
    return 100;
  if (voltage_mv <= curve[6].mv)
    return 0;

  /* Interpolate between curve points */
  for (int i = 0; i < 6; i++) {
    if (voltage_mv >= curve[i + 1].mv) {
      uint16_t v_high = curve[i].mv;
      uint16_t v_low = curve[i + 1].mv;
      uint8_t s_high = curve[i].soc;
      uint8_t s_low = curve[i + 1].soc;

      /* Linear interpolation: SoC = s_low + (v - v_low) * (s_high - s_low) / (v_high - v_low) */
      uint32_t soc =
          s_low + ((uint32_t)(voltage_mv - v_low) * (s_high - s_low)) /
                      (v_high - v_low);

      return (uint8_t)soc;
    }
  }

  return 0;
}

/* Enable motor current measurements */
void SensorTask_StartMotorMeasurements(void) {
  taskENTER_CRITICAL();
  s_motor_measurements_enabled = true;
  taskEXIT_CRITICAL();
}

/* Disable motor current measurements */
void SensorTask_StopMotorMeasurements(void) {
  taskENTER_CRITICAL();
  s_motor_measurements_enabled = false;
  taskEXIT_CRITICAL();
}

/* Main sensor measurement task
   Acquires ADC samples, performs calculations, updates sensor values via mutex */
void StartSensorTask(void *argument) {
  SensorTaskArgsTypeDef *args = (SensorTaskArgsTypeDef *)argument;

  /* Receive config access and sensor values access handles from main */
  if (args != NULL) {
    if (args->config_access != NULL) {
      s_config_access = args->config_access;
    }
    if (args->sensor_values_access != NULL) {
      s_sensor_values_access = args->sensor_values_access;
    }
  }

#if OS_TASKS_DEBUG
  printf("SensorTask running (heap=%lu)\n",
         (unsigned long)xPortGetFreeHeapSize());
#endif

  if (s_sensor_values_access == NULL || s_sensor_values_access->mutex == NULL) {
    Error_Handler();
  }

  /* Calibrate ADC and start DMA conversions */
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) {
    Error_Handler();
  }

  memset(s_adc_dma_buffer, 0, sizeof(s_adc_dma_buffer));
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)s_adc_dma_buffer,
                        SENSOR_TASK_ADC_CHANNEL_COUNT) != HAL_OK) {
    Error_Handler();
  }

  /* Wait for first ADC conversions to complete before starting main loop */
  osDelay(safe_ms_to_ticks(SENSOR_TASK_MIN_SAMPLING_PERIOD_MS));

  TickType_t last_wake_time = osKernelGetTickCount();
  const uint16_t temp_cycle_threshold = TEMP_MEAS_PER_MOTOR_MEAS_CYCLES;
  uint32_t temp_measurement_counter =
      temp_cycle_threshold; /* Trigger immediate measurement */

  printf("SensorTask init OK. Running loop...\n");

  for (;;) {
    /* Perform all ADC calculations OUTSIDE the mutex (keep critical section short) */
    const uint16_t vref_raw = s_adc_dma_buffer[SENSOR_TASK_VREF_CHANNEL_INDEX];
    const uint32_t vref_mv = calculate_vref_voltage(vref_raw);

    float temperature = 0.0f;
    float battery_voltage = 0.0f;
    float motor_current = 0.0f;
    uint8_t battery_soc = 0U;
    bool update_motor = false;
    bool update_temp_bat = false;

    /* Check if motor measurements are enabled */
    bool local_motor_enabled;
    taskENTER_CRITICAL();
    local_motor_enabled = s_motor_measurements_enabled;
    taskEXIT_CRITICAL();

    if (local_motor_enabled) {
      /* Motor measurements enabled: sample motor current */
      const uint16_t motor_raw =
          s_adc_dma_buffer[SENSOR_TASK_MOTOR_CHANNEL_INDEX];
      const float motor_voltage = convert_raw_to_voltage(motor_raw, vref_mv);
      motor_current = motor_voltage / SENSOR_TASK_MOTOR_SHUNT_OHMS;
      update_motor = true;

      /* Check if it's time to measure temperature and battery */
      if (temp_cycle_threshold == 0U ||
          temp_measurement_counter >= temp_cycle_threshold) {
        temp_measurement_counter = 0U;
        const uint16_t temp_raw =
            s_adc_dma_buffer[SENSOR_TASK_TEMPERATURE_CHANNEL_INDEX];
        const uint16_t vbat_raw =
            s_adc_dma_buffer[SENSOR_TASK_VBAT_CHANNEL_INDEX];

        temperature = calculate_temperature(temp_raw, vref_mv);
        battery_voltage = convert_raw_to_voltage(vbat_raw, vref_mv) *
                          SENSOR_TASK_VBAT_DIVIDER;
        battery_soc = calculate_battery_soc(battery_voltage);
        update_temp_bat = true;
#if SENSOR_TASK_DEBUG_PRINTING
        printf("SensorTask: vref_raw=%u, temp_raw=%u, vbat_raw=%u, "
               "vref_mv=%lu, battery_soc=%u%%\n",
               vref_raw, temp_raw, vbat_raw, (unsigned long)vref_mv,
               battery_soc);
#endif
      }
      temp_measurement_counter += 1U;
    } else {
      /* Motor measurements disabled: always measure temperature and battery */
      const uint16_t temp_raw =
          s_adc_dma_buffer[SENSOR_TASK_TEMPERATURE_CHANNEL_INDEX];
      const uint16_t vbat_raw =
          s_adc_dma_buffer[SENSOR_TASK_VBAT_CHANNEL_INDEX];

      temperature = calculate_temperature(temp_raw, vref_mv);
      battery_voltage =
          convert_raw_to_voltage(vbat_raw, vref_mv) * SENSOR_TASK_VBAT_DIVIDER;
      battery_soc = calculate_battery_soc(battery_voltage);
      update_temp_bat = true;
      temp_measurement_counter = 0U; /* Reset counter */
#if SENSOR_TASK_DEBUG_PRINTING
      printf("SensorTask: vref_raw=%u, temp_raw=%u, vbat_raw=%u, vref_mv=%lu, "
             "battery_soc=%u%%\n",
             vref_raw, temp_raw, vbat_raw, (unsigned long)vref_mv, battery_soc);
#endif
    }

    /* Update sensor values via mutex */
    if (osMutexAcquire(s_sensor_values_access->mutex, osWaitForever) == osOK) {
      if (update_motor) {
        s_sensor_values_access->data.motor_current = motor_current;
      }

      if (update_temp_bat) {
        s_sensor_values_access->data.ambient_temperature = temperature;
        s_sensor_values_access->data.soc = battery_soc;
#if DRIVER_TEST
        s_sensor_values_access->data.battery_voltage = battery_voltage;
#endif
      }
      osMutexRelease(s_sensor_values_access->mutex);
    }

    /* Determine delay interval based on motor measurement state */
    TickType_t task_interval;
    if (local_motor_enabled) {
      /* Motor measurements enabled: use standard motor measurement period */
      task_interval = safe_ms_to_ticks(MOTOR_MEAS_PERIOD_MS);
    } else {
      /* Motor measurements disabled: measure temp/battery once per minute */
      task_interval = safe_ms_to_ticks(TEMPERATURE_AND_BAT_MEAS_PERIOD_MS);
    }
    vTaskDelayUntil(&last_wake_time, task_interval);
  }
}