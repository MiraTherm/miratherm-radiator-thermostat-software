#include "maintenance_task.h"
#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include <stdlib.h>

/* Minimal Maintenance mock: wait for adapt start, delay, then report end back to System */
void StartMaintenanceTask(void *argument)
{
  MaintenanceTaskArgsTypeDef *args = (MaintenanceTaskArgsTypeDef *)argument;
  if (args == NULL) {
    printf("ERROR: Maintenance task args NULL\n");
    Error_Handler();
  }
  osMessageQueueId_t s2m_q = args->system2_maint_queue;
  osMessageQueueId_t m2s_q = args->maint2_system_queue;

#if OS_TASKS_DEBUG
  printf("MaintenanceTask running (mock)\n");
#endif

  System2MaintEventTypeDef s2m;
  for(;;) {
    if (s2m_q != NULL && osMessageQueueGet(s2m_q, &s2m, NULL, osWaitForever) == osOK) {
      if (s2m == EVT_ADAPT_START) {
        /* Simulate long-running adaptation */
        osDelay(pdMS_TO_TICKS(10000));
        /* Randomize result */
        int success = (rand() % 2) == 0;
        Maint2SystemEvent_t m2s = {EVT_ADAPT_END, success ? OK : FAIL};
        if (m2s_q != NULL) {
          osMessageQueuePut(m2s_q, &m2s, 0, 0);
        }
      }
    }
  }
}