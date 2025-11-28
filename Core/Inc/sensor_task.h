#ifndef CORE_SENSOR_TASK_H
#define CORE_SENSOR_TASK_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SENSOR_TASK_DEFAULT_MOTOR_PERIOD_MS 100U
#define SENSOR_TASK_DEFAULT_TEMP_BATTERY_PERIOD_MS 500U

typedef struct
{
  float CurrentTemp;
  float BatteryVoltage;
  float MotorCurrent;
} SensorValuesTypeDef;

void StartSensorTask(void *argument);

bool SensorValues_Copy(SensorValuesTypeDef *dest);

uint32_t SensorTask_GetMotorMeasurementPeriodMs(void);
uint32_t SensorTask_GetTempBatteryMeasurementPeriodMs(void);

void SensorTask_SetMotorMeasurementPeriodMs(uint32_t period_ms);
void SensorTask_SetTempBatteryMeasurementPeriodMs(uint32_t period_ms);

void SensorTask_SetTemperatureCalibrationOffset(float offset_c);
float SensorTask_GetTemperatureCalibrationOffset(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_SENSOR_TASK_H */
