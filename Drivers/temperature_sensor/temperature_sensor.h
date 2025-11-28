/*
 * Minimal driver for the internal STM32 temperature sensor and reference voltage.
 */
#ifndef DRIVERS_TEMPERATURE_SENSOR_H
#define DRIVERS_TEMPERATURE_SENSOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Triggers the DMA-driven conversion sequence and returns the temperature sensor raw sample.
 * VrefINT is measured alongside TEMPSENSOR and discarded; the caller does not manage ADC start/stop.
 */
uint16_t TemperatureSensor_ReadRaw(void);

/**
 * Converts the latest DMA samples into degrees Celsius.
 * Uses the factory temperature calibration values, the measured VrefINT voltage, and the configured offset.
 */
float TemperatureSensor_GetCelsius(void);

/**
 * Adjusts the compensation added to the calculated temperature.
 */
void TemperatureSensor_SetCalibrationOffset(float offset);

/**
 * Returns the current calibration offset.
 */
float TemperatureSensor_GetCalibrationOffset(void);

/**
 * Returns the moving average of the last 20 temperature readings in degrees Celsius.
 */
float TemperatureSensor_GetMovingAverage(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_TEMPERATURE_SENSOR_H */