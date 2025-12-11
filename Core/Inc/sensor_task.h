#ifndef CORE_SENSOR_TASK_H
#define CORE_SENSOR_TASK_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SENSOR_TASK_MIN_SAMPLING_PERIOD_MS 26U  /* 4 channels * (12.5 ADC cycles + 640.5 oversampling)  * 256 oversample / 32MHz ≈ 20.896 ms + 5 ms margin */
#define MOTOR_MEAS_PERIOD_MS 100U
#define TEMPERATURE_AND_BAT_MEAS_PERIOD_MS 10000U
#define TEMP_MEAS_PER_MOTOR_MEAS_CYCLES (TEMPERATURE_AND_BAT_MEAS_PERIOD_MS / MOTOR_MEAS_PERIOD_MS)

typedef struct
{
  float CurrentTemp;
  float BatteryVoltage;
  float MotorCurrent;
} SensorValuesTypeDef;

void StartSensorTask(void *argument);

bool SensorTask_CopySensorValues(SensorValuesTypeDef *dest);

uint32_t SensorTask_GetMotorMeasurementPeriodMs(void);
uint32_t SensorTask_GetTempBatteryMeasurementPeriodMs(void);

void SensorTask_SetTemperatureCalibrationOffset(float offset_c);
float SensorTask_GetTemperatureCalibrationOffset(void);

void SensorTask_StartMotorMeasurements(void);
void SensorTask_StopMotorMeasurements(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_SENSOR_TASK_H */
