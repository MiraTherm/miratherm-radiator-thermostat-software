#ifndef CORE_INC_VIEW_PRESENTER_TASK_H
#define CORE_INC_VIEW_PRESENTER_TASK_H

#include "input_task.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VP_TASK_STACK_SIZE                                                     \
  (1024U * 4U) // Important: Too small stack will cause hard faults!

#include "sensor_task.h"
#include "storage_task.h"
#include "system_task.h"

typedef struct {
  osMessageQueueId_t input2vp_event_queue;
  osMessageQueueId_t
      vp2system_event_queue; /* For sending user actions to System task */
  osMessageQueueId_t
      system2vp_event_queue; /* For receiving events from System task */
  SystemContextAccessTypeDef
      *system_context_access; /* Pointer to system context for UI updates */
  ConfigAccessTypeDef *config_access; /* Pointer to config access */
  SensorValuesAccessTypeDef
      *sensor_values_access; /* Pointer to sensor values */
} ViewPresenterTaskArgsTypeDef;

void StartViewPresenterTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEW_PRESENTER_TASK_H */
