#ifndef TESTS_H
#define TESTS_H

#include "cmsis_os2.h"
#include "storage_task.h"

#ifdef __cplusplus
extern "C" {
#endif

// Tests settings
#define TESTS               1
#define DRIVER_TEST         1
#define ADAPTATION_TEST     0

#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DRIVER_TEST
struct SensorValuesAccessTypeDef;
struct ConfigAccessTypeDef;
void Driver_Test(osMessageQueueId_t storage2system_event_queue, ConfigAccessTypeDef *config_access, struct SensorValuesAccessTypeDef *sensor_values_access);
#elif ADAPTATION_TEST
void Adaptation_Test(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* TESTS_H */