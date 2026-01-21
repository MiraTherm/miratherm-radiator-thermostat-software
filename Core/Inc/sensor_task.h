/**
 * @file sensor_task.h
 * @brief Sensor measurement and ADC data acquisition task
 * @details Manages STM32WB55 analog-to-digital conversion for temperature,
 *          battery voltage, motor current, and internal voltage reference sampling.
 *          Uses DMA-based ADC conversion with FreeRTOS task scheduling. Includes
 *          battery state-of-charge calculation based on 2-cell alkaline discharge
 *          curves and temperature offset calibration support.
 * @see sensor_task.c for implementation
 */

#ifndef CORE_SENSOR_TASK_H
#define CORE_SENSOR_TASK_H

#include <stdint.h>
#include <stdbool.h>

#include "tests.h"
#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def SENSOR_TASK_MIN_SAMPLING_PERIOD_MS
 * @brief Minimum ADC sampling period in milliseconds
 * @details Calculated as: 4 channels * (12.5 ADC cycles + 640.5 oversampling)
 *          * 256 oversample / 32MHz ≈ 20.896 ms + 5 ms margin for safety
 */
#define SENSOR_TASK_MIN_SAMPLING_PERIOD_MS 26U

/**
 * @def MOTOR_MEAS_PERIOD_MS
 * @brief Motor current measurement interval in milliseconds
 * @details When motor measurements are enabled, ADC samples at this period.
 *          Allows transient current detection during motor operation.
 */
#define MOTOR_MEAS_PERIOD_MS 100U

/**
 * @def TEMPERATURE_AND_BAT_MEAS_PERIOD_MS
 * @brief Temperature and battery measurement period in milliseconds
 * @details When motor measurements are disabled, temperature and battery
 *          are sampled at this interval (10 seconds).
 */
#define TEMPERATURE_AND_BAT_MEAS_PERIOD_MS 10000U

/**
 * @def TEMP_MEAS_PER_MOTOR_MEAS_CYCLES
 * @brief Ratio of temperature measurements per motor measurement cycle
 * @details When motor measurements are enabled, temperature and battery are
 *          sampled every N motor measurement cycles to reduce ADC load.
 *          Derived from TEMPERATURE_AND_BAT_MEAS_PERIOD_MS / MOTOR_MEAS_PERIOD_MS
 */
#define TEMP_MEAS_PER_MOTOR_MEAS_CYCLES (TEMPERATURE_AND_BAT_MEAS_PERIOD_MS / MOTOR_MEAS_PERIOD_MS)

/**
 * @typedef SensorValuesTypeDef
 * @brief Aggregated sensor measurement values
 * @details Contains calibrated sensor readings: temperature (with offset applied),
 *          battery state-of-charge percentage, motor current (during active measurement),
 *          and battery voltage (when DRIVER_TEST enabled for debugging).
 * @see SensorValuesAccessTypeDef for thread-safe access wrapper
 */
typedef struct
{
  float CurrentTemp;       /**< Current temperature in °C (with offset applied) */
#if DRIVER_TEST
  float BatteryVoltage;    /**< Battery voltage in V (debug only) */
#endif
  uint8_t SoC;             /**< Battery state-of-charge percentage (0-100%) */
  float MotorCurrent;      /**< Motor shunt current in amperes */
} SensorValuesTypeDef;

/**
 * @typedef SensorValuesAccessTypeDef
 * @brief Thread-safe access wrapper for sensor measurement values
 * @details Provides osMutex-protected access to sensor readings. Must be
 *          acquired before reading SensorValuesTypeDef to ensure consistency.
 *          Task-safe for concurrent access from multiple FreeRTOS tasks.
 * @see StartSensorTask for task initialization
 */
typedef struct SensorValuesAccessTypeDef
{
  osMutexId_t mutex;       /**< CMSIS-RTOS2 mutex for thread-safe access */
  SensorValuesTypeDef data; /**< Sensor measurement values */
} SensorValuesAccessTypeDef;

/**
 * @brief Start the sensor measurement task
 * @details Initializes ADC with DMA buffer, calibrates analog reference,
 *          and enters infinite measurement loop. Task manages all ADC sampling,
 *          calculations, and updates to shared sensor values through mutex.
 * @param argument Pointer to SensorTaskArgsTypeDef containing config_access
 *                 and sensor_values_access pointers. NULL argument causes
 *                 Error_Handler() to be called.
 * @return Does not return; runs as infinite FreeRTOS task
 * @see SensorTaskArgsTypeDef, SensorValuesAccessTypeDef
 */
void StartSensorTask(void *argument);

/**
 * @brief Enable motor current measurement sampling
 * @details Allows sensor task to include motor current in ADC conversion
 *          sequence. Used when motor is active to monitor shunt voltage.
 *          Reduces temperature/battery sampling rate to TEMP_MEAS_PER_MOTOR_MEAS_CYCLES.
 * @note Thread-safe; uses taskENTER_CRITICAL/taskEXIT_CRITICAL
 * @see SensorTask_StopMotorMeasurements
 */
void SensorTask_StartMotorMeasurements(void);

/**
 * @brief Disable motor current measurement sampling
 * @details Stops motor current ADC sampling and increases temperature/battery
 *          sampling rate to TEMPERATURE_AND_BAT_MEAS_PERIOD_MS (every 10 seconds).
 *          Reduces ADC/DMA traffic when motor is idle.
 * @note Thread-safe; uses taskENTER_CRITICAL/taskEXIT_CRITICAL
 * @see SensorTask_StartMotorMeasurements
 */
void SensorTask_StopMotorMeasurements(void);

/**
 * @def SENSOR_TASK_STACK_SIZE
 * @brief Stack size in bytes for the sensor measurement task
 * @details Allocated from FreeRTOS heap during task creation
 */
#define SENSOR_TASK_STACK_SIZE (512U * 4U)

#ifdef __cplusplus
}
#endif

#endif /* CORE_SENSOR_TASK_H */
