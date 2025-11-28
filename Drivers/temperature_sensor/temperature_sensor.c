#include "temperature_sensor.h"

#include <stdbool.h>
#include "stm32wbxx_hal.h"
#include "stm32wbxx_ll_adc.h"

#define TEMPERATURE_SENSOR_DMA_CONVERSIONS 2U
#define TEMPERATURE_SENSOR_DMA_TEMP_INDEX 0U
#define TEMPERATURE_SENSOR_DMA_VREF_INDEX 1U
#define TEMPERATURE_SENSOR_DMA_TIMEOUT_MS 10U
#define TEMPERATURE_SENSOR_MOVING_AVG_SIZE 20U

static float g_calibration_offset_c = 0.0f;
static uint16_t g_adc_dma_buffer[TEMPERATURE_SENSOR_DMA_CONVERSIONS];
static float g_temperature_buffer[TEMPERATURE_SENSOR_MOVING_AVG_SIZE] = {0.0f};
static uint8_t g_buffer_index = 0U;
static uint8_t g_buffer_count = 0U;

extern ADC_HandleTypeDef hadc1;

static bool capture_temperature_sensor_samples(uint16_t *temperature_raw, uint16_t *vref_raw)
{
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)g_adc_dma_buffer,
                        TEMPERATURE_SENSOR_DMA_CONVERSIONS) != HAL_OK)
  {
    return false;
  }

  /* Sampling the internal channels requires the slowest window (640.5 cycles).
   * With ADC clock set to 64MHz/64, each sequence takes at least
   * 640.5/(64MHz/64) seconds before the data is ready.
   */
  const HAL_StatusTypeDef status = HAL_ADC_PollForConversion(&hadc1, TEMPERATURE_SENSOR_DMA_TIMEOUT_MS);

  (void)HAL_ADC_Stop_DMA(&hadc1);
  (void)HAL_ADC_Stop(&hadc1);

  if (status != HAL_OK)
  {
    return false;
  }

  if (temperature_raw != NULL)
  {
    *temperature_raw = g_adc_dma_buffer[TEMPERATURE_SENSOR_DMA_TEMP_INDEX];
  }

  if (vref_raw != NULL)
  {
    *vref_raw = g_adc_dma_buffer[TEMPERATURE_SENSOR_DMA_VREF_INDEX];
  }

  return true;
}

static float temperature_from_samples(uint16_t raw, uint16_t vref_raw)
{
  uint32_t vref_mv = TEMPSENSOR_CAL_VREFANALOG;
  vref_mv = __LL_ADC_CALC_VREFANALOG_VOLTAGE(vref_raw, LL_ADC_RESOLUTION_12B);

  const int32_t temperature = __LL_ADC_CALC_TEMPERATURE(vref_mv,
                                                        raw,
                                                        LL_ADC_RESOLUTION_12B);
  return (float)temperature + g_calibration_offset_c;
}

uint16_t TemperatureSensor_ReadRaw(void)
{
  uint16_t raw = 0U;
  if (!capture_temperature_sensor_samples(&raw, NULL))
  {
    return 0U;
  }
  return raw;
}

float TemperatureSensor_GetCelsius(void)
{
  uint16_t raw = 0U;
  uint16_t vref_raw = 0U;

  if (!capture_temperature_sensor_samples(&raw, &vref_raw))
  {
    return 0.0f;
  }

  const float temperature = temperature_from_samples(raw, vref_raw);

  /* Add temperature to moving average buffer */
  g_temperature_buffer[g_buffer_index] = temperature;
  g_buffer_index = (g_buffer_index + 1U) % TEMPERATURE_SENSOR_MOVING_AVG_SIZE;
  if (g_buffer_count < TEMPERATURE_SENSOR_MOVING_AVG_SIZE)
  {
    g_buffer_count++;
  }

  return temperature;
}

void TemperatureSensor_SetCalibrationOffset(float offset)
{
  g_calibration_offset_c = offset;
}

float TemperatureSensor_GetCalibrationOffset(void)
{
  return g_calibration_offset_c;
}

float TemperatureSensor_GetMovingAverage(void)
{
  if (g_buffer_count == 0U)
  {
    return 0.0f;
  }

  float sum = 0.0f;
  for (uint8_t i = 0U; i < g_buffer_count; i++)
  {
    sum += g_temperature_buffer[i];
  }

  return sum / (float)g_buffer_count;
}