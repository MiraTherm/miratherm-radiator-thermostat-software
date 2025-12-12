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
	EVT_CFG_LOAD_END = 0
} Storage2SystemEventTypeDef;

typedef struct
{
  float TemperatureOffsetC;
} ConfigTypeDef;

/**
 * @brief Configuration access wrapper with mutex protection
 */
typedef struct
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
