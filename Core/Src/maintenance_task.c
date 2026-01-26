/**
 ******************************************************************************
 * @file           :  maintenance_task.c
 * @brief          :  Implementation of system maintenance and adaptation task
 *
 * @details        :  Handles radiator adaptation and calibration procedures
 *                    triggered by the system task. Currently provides mock
 *                    implementation for development and testing.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#include "maintenance_task.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

/* Main maintenance task: process adaptation and maintenance commands */
void StartMaintenanceTask(void *argument) {
  MaintenanceTaskArgsTypeDef *args = (MaintenanceTaskArgsTypeDef *)argument;
  if (args == NULL) {
    printf("ERROR: Maintenance task args NULL\n");
    Error_Handler();
  }
  osMessageQueueId_t s2m_q = args->system2maint_event_queue;
  osMessageQueueId_t m2s_q = args->maint2system_event_queue;

#if OS_TASKS_DEBUG
  printf("MaintenanceTask running (mock)\n");
#endif

  /* Wait for maintenance commands and process them */
  System2MaintEventTypeDef s2m;
  for (;;) {
    if (s2m_q != NULL &&
        osMessageQueueGet(s2m_q, &s2m, NULL, osWaitForever) == osOK) {
      if (s2m == EVT_ADAPT_START) {
        /* Simulate long-running radiator adaptation procedure */
        osDelay(pdMS_TO_TICKS(10000));
        
        /* Randomize result for testing (TODO: implement real adaptation) */
        int success = (rand() % 2) == 0;
        Maint2SystemEvent_t m2s = {EVT_ADAPT_END, success ? OK : FAIL};
        
        /* Report result back to system task */
        if (m2s_q != NULL) {
          osMessageQueuePut(m2s_q, &m2s, 0, 0);
        }
      }
    }
  }
}