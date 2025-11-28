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
static osMutexId_t s_values_mutex;
static volatile uint32_t s_motor_period_ms = SENSOR_TASK_DEFAULT_MOTOR_PERIOD_MS;
static volatile uint32_t s_temp_period_ms = SENSOR_TASK_DEFAULT_TEMP_BATTERY_PERIOD_MS;
static volatile float s_temperature_offset_c = 0.0f;

extern ADC_HandleTypeDef hadc1;

static TickType_t safe_ms_to_ticks(uint32_t ms)
{
  const uint32_t safe_ms = (ms == 0U) ? 1U : ms;
  return pdMS_TO_TICKS(safe_ms);
}

static uint32_t enforce_min_sampling_period_ms(uint32_t period_ms)
{
  /* ADC channels are configured with 640.5 cycles and 256× oversampling at 64MHz/64, so each measurement takes
   * at least 640.5 * 256 / (64MHz / 64) ≈ 163.968 ms. Don't allow faster measurement requests than that. */
  if (period_ms < SENSOR_TASK_MIN_SAMPLING_PERIOD_MS)
  {
    return SENSOR_TASK_MIN_SAMPLING_PERIOD_MS;
  }
  return period_ms;
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

bool SensorValues_Copy(SensorValuesTypeDef *dest)
{
  if (dest == NULL || s_values_mutex == NULL)
  {
    return false;
  }

  if (osMutexAcquire(s_values_mutex, osWaitForever) != osOK)
  {
    return false;
  }

  *dest = s_sensor_values;
  osMutexRelease(s_values_mutex);
  return true;
}

uint32_t SensorTask_GetMotorMeasurementPeriodMs(void)
{
  return s_motor_period_ms;
}

uint32_t SensorTask_GetTempBatteryMeasurementPeriodMs(void)
{
  return s_temp_period_ms;
}

void SensorTask_SetMotorMeasurementPeriodMs(uint32_t period_ms)
{
  if (period_ms == 0U)
  {
    period_ms = 1U;
  }

  period_ms = enforce_min_sampling_period_ms(period_ms);

  taskENTER_CRITICAL();
  s_motor_period_ms = period_ms;
  taskEXIT_CRITICAL();
}

void SensorTask_SetTempBatteryMeasurementPeriodMs(uint32_t period_ms)
{
  if (period_ms == 0U)
  {
    period_ms = 1U;
  }

  period_ms = enforce_min_sampling_period_ms(period_ms);

  taskENTER_CRITICAL();
  s_temp_period_ms = period_ms;
  taskEXIT_CRITICAL();
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
  s_values_mutex = osMutexNew(&mutex_attr);
  if (s_values_mutex == NULL)
  {
    Error_Handler();
  }

  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)s_adc_dma_buffer, SENSOR_TASK_ADC_CHANNEL_COUNT) != HAL_OK)
  {
    Error_Handler();
  }

  TickType_t last_motor_tick = osKernelGetTickCount();
  TickType_t last_temp_tick = last_motor_tick;

  for (;;)
  {
    const TickType_t now = osKernelGetTickCount();
    const TickType_t motor_interval = safe_ms_to_ticks(s_motor_period_ms);

    if ((now - last_motor_tick) >= motor_interval)
    {
      const uint16_t vref_raw = s_adc_dma_buffer[SENSOR_TASK_VREF_CHANNEL_INDEX];
      const uint32_t vref_mv = calculate_vref_voltage(vref_raw);
      const uint16_t motor_raw = s_adc_dma_buffer[SENSOR_TASK_MOTOR_CHANNEL_INDEX];
      const float motor_voltage = convert_raw_to_voltage(motor_raw, vref_mv);
      const float motor_current = motor_voltage / SENSOR_TASK_MOTOR_SHUNT_OHMS;

      if (osMutexAcquire(s_values_mutex, osWaitForever) == osOK)
      {
        s_sensor_values.MotorCurrent = motor_current;
        osMutexRelease(s_values_mutex);
      }

      last_motor_tick = now;
    }

    const TickType_t temp_interval = safe_ms_to_ticks(s_temp_period_ms);
    if ((now - last_temp_tick) >= temp_interval)
    {
      const uint16_t vref_raw = s_adc_dma_buffer[SENSOR_TASK_VREF_CHANNEL_INDEX];
      const uint32_t vref_mv = calculate_vref_voltage(vref_raw);
      const uint16_t temp_raw = s_adc_dma_buffer[SENSOR_TASK_TEMPERATURE_CHANNEL_INDEX];
      const uint16_t vbat_raw = s_adc_dma_buffer[SENSOR_TASK_VBAT_CHANNEL_INDEX];
      const float temperature = calculate_temperature(temp_raw, vref_mv);
      const float battery_voltage = convert_raw_to_voltage(vbat_raw, vref_mv) * SENSOR_TASK_VBAT_DIVIDER;

      if (osMutexAcquire(s_values_mutex, osWaitForever) == osOK)
      {
        s_sensor_values.CurrentTemp = temperature;
        s_sensor_values.BatteryVoltage = battery_voltage;
        osMutexRelease(s_values_mutex);
      }

      last_temp_tick = now;
    }

    osDelay(pdMS_TO_TICKS(5U));
  }
}
