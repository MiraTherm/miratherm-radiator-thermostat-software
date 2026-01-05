#ifndef CORE_STORAGE_TASK_H
#define CORE_STORAGE_TASK_H

#include <stdint.h>
#include <stdbool.h>

#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	EVT_CFG_LOAD_END = 0,
  EVT_CFG_RST_END
} Storage2SystemEventTypeDef;

typedef enum
{
    EVT_CFG_RST_REQ = 0
} System2StorageEventTypeDef;

typedef struct
{
    uint8_t StartHour;
    uint8_t StartMinute;
    uint8_t EndHour;
    uint8_t EndMinute;
    float Temperature;
} TimeSlotTypeDef;

typedef struct
{
    uint8_t NumTimeSlots;
    TimeSlotTypeDef TimeSlots[5];
} DailyScheduleTypeDef;

typedef struct
{
  float TemperatureOffsetC;
  DailyScheduleTypeDef DailySchedule;
  float ManualTargetTemp;  /* Manual mode target temperature (persistent storage) */
} ConfigTypeDef;

/**
 * @brief Configuration access wrapper with mutex protection
 */
typedef struct ConfigAccessTypeDef
{
  osMutexId_t mutex;
  ConfigTypeDef data;
} ConfigAccessTypeDef;

void StartStorageTask(void *argument);

/**
 * Try to get a storage system event.
 * Thread-safe via internal queue.
 */
bool StorageTask_TryGetEvent(Storage2SystemEventTypeDef *event, uint32_t timeout_ticks);

#define STORAGE_TASK_STACK_SIZE (512U * 4U)

#ifdef __cplusplus
}
#endif

#endif /* CORE_STORAGE_TASK_H */
