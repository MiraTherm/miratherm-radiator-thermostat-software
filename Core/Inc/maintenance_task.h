#ifndef CORE_INC_MAINTENANCE_TASK_H
#define CORE_INC_MAINTENANCE_TASK_H

#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAINT_TASK_STACK_SIZE (512U * 4U)

/* System -> Maint events */
typedef enum {
  EVT_ADAPT_START = 0
} System2MaintEventTypeDef;

/* Maint -> System event result */
typedef enum {
  OK = 0,
  FAIL
} MaintResultTypeDef;

/* Maint -> System events */
typedef enum
{
    EVT_ADAPT_END
} Maint2SystemEventTypeDef;

typedef struct
{
    Maint2SystemEventTypeDef type;
    MaintResultTypeDef result;
} Maint2SystemEvent_t;

/* Arguments passed to StartMaintTask */
typedef struct {
  osMessageQueueId_t system2_maint_queue;    /* System -> Maint */
  osMessageQueueId_t maint2_system_queue;    /* Maint -> System */
} MaintenanceTaskArgsTypeDef;

void StartMaintenanceTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_MAINTENANCE_TASK_H */
