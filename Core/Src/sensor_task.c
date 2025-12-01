#include "sensor_task.h"

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "stm32wbxx_hal.h"
#include "stm32wbxx_ll_adc.h"

#include <stddef.h>

#define SENSOR_TASK_ADC_CHANNEL_COUNT 4U
#define SENSOR_TASK_VREF_CHANNEL_INDEX 0U
#define SENSOR_TASK_MOTOR_CHANNEL_INDEX 1U
#define SENSOR_TASK_TEMPERATURE_CHANNEL_INDEX 2U
#define SENSOR_TASK_VBAT_CHANNEL_INDEX 3U
#define SENSOR_TASK_MOTOR_SHUNT_OHMS 1.0f
#define SENSOR_TASK_VBAT_DIVIDER 3.0f

static volatile uint16_t s_adc_dma_buffer[SENSOR_TASK_ADC_CHANNEL_COUNT];
static SensorValuesTypeDef s_sensor_values = {0.0f, 0.0f, 0.0f};
static osMutexId_t s_sensor_values_mutex;
static volatile float s_temperature_offset_c = 0.0f;

extern ADC_HandleTypeDef hadc1;

static TickType_t safe_ms_to_ticks(uint32_t ms)
{
  const uint32_t safe_ms = (ms == 0U) ? 1U : ms;
  return pdMS_TO_TICKS(safe_ms);
}

static uint32_t calculate_vref_voltage(uint16_t vref_raw)
{
  if (vref_raw == 0U)
  {
    return TEMPSENSOR_CAL_VREFANALOG;
  }
  return __LL_ADC_CALC_VREFANALOG_VOLTAGE(vref_raw, LL_ADC_RESOLUTION_12B);
}

static float convert_raw_to_voltage(uint16_t raw_value, uint32_t vref_mv)
{
  const uint32_t millivolt = __LL_ADC_CALC_DATA_TO_VOLTAGE(vref_mv, raw_value, LL_ADC_RESOLUTION_12B);
  return (float)millivolt * 0.001f;
}

static float calculate_temperature(uint16_t temperature_raw, uint32_t vref_mv)
{
  const int32_t temperature = __LL_ADC_CALC_TEMPERATURE(vref_mv, temperature_raw, LL_ADC_RESOLUTION_12B);
  return (float)temperature + s_temperature_offset_c;
}

bool SensorTask_CopySensorValues(SensorValuesTypeDef *dest)
{
  if (dest == NULL || s_sensor_values_mutex == NULL)
  {
    return false;
  }

  if (osMutexAcquire(s_sensor_values_mutex, osWaitForever) != osOK)
  {
    return false;
  }

  *dest = s_sensor_values;
  osMutexRelease(s_sensor_values_mutex);
  return true;
}

void SensorTask_SetTemperatureCalibrationOffset(float offset_c)
{
  taskENTER_CRITICAL();
  s_temperature_offset_c = offset_c;
  taskEXIT_CRITICAL();
}

float SensorTask_GetTemperatureCalibrationOffset(void)
{
  float offset;
  taskENTER_CRITICAL();
  offset = s_temperature_offset_c;
  taskEXIT_CRITICAL();
  return offset;
}

void StartSensorTask(void *argument)
{
  (void)argument;

  const osMutexAttr_t mutex_attr = {
    .name = "SensorValues",
  };
  s_sensor_values_mutex = osMutexNew(&mutex_attr);
  if (s_sensor_values_mutex == NULL)
  {
    Error_Handler();
  }

  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)s_adc_dma_buffer, SENSOR_TASK_ADC_CHANNEL_COUNT) != HAL_OK)
  {
    Error_Handler();
  }

  TickType_t last_wake_time = osKernelGetTickCount();
  uint32_t temp_measurement_counter = 0U;

  for (;;)
  {
    const TickType_t motor_interval = safe_ms_to_ticks(MOTOR_MEAS_PERIOD_MS);
    vTaskDelayUntil(&last_wake_time, motor_interval);

    /* 1. Perform all ADC calculations OUTSIDE the mutex (keep critical section short) */
    const uint16_t vref_raw = s_adc_dma_buffer[SENSOR_TASK_VREF_CHANNEL_INDEX];
    const uint32_t vref_mv = calculate_vref_voltage(vref_raw);
    const uint16_t motor_raw = s_adc_dma_buffer[SENSOR_TASK_MOTOR_CHANNEL_INDEX];
    const float motor_voltage = convert_raw_to_voltage(motor_raw, vref_mv);
    const float motor_current = motor_voltage / SENSOR_TASK_MOTOR_SHUNT_OHMS;

    float temperature = 0.0f;
    float battery_voltage = 0.0f;
    bool update_temp_bat = false;

    temp_measurement_counter += 1U;
    const uint32_t temp_cycle_threshold = TEMP_MEAS_PER_MOTOR_MEAS_CYCLES;
    
    if (temp_cycle_threshold == 0U || temp_measurement_counter >= temp_cycle_threshold)
    {
      temp_measurement_counter = 0U;
      const uint16_t temp_raw = s_adc_dma_buffer[SENSOR_TASK_TEMPERATURE_CHANNEL_INDEX];
      const uint16_t vbat_raw = s_adc_dma_buffer[SENSOR_TASK_VBAT_CHANNEL_INDEX];
      
      temperature = calculate_temperature(temp_raw, vref_mv);
      battery_voltage = convert_raw_to_voltage(vbat_raw, vref_mv) * SENSOR_TASK_VBAT_DIVIDER;
      update_temp_bat = true;
    }

    /* 2. Single Mutex Acquire for all updates */
    if (osMutexAcquire(s_sensor_values_mutex, osWaitForever) == osOK)
    {
      s_sensor_values.MotorCurrent = motor_current;
      
      if (update_temp_bat)
      {
        s_sensor_values.CurrentTemp = temperature;
        s_sensor_values.BatteryVoltage = battery_voltage;
      }
      osMutexRelease(s_sensor_values_mutex);
    }
  }
}