/**
 ******************************************************************************
 * @file           :  system_task.c
 * @brief          :  Implementation of main system control task
 *
 * @details        :  Manages system task initialization and the main control
 *                    loop driving the state machine. Coordinates event queues
 *                    and provides global context access for API helpers.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#include "system_task.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "maintenance_task.h"
#include "storage_task.h"
#include "system_state_machine.h"
#include <stdio.h>

/* Global pointer to system context for API helpers (System_GetState, etc.) */
static SystemContextAccessTypeDef *g_sys_ctx = NULL;

/* Main system task: initializes state machine and runs control loop */
void StartSystemTask(void *argument) {
  SystemTaskArgsTypeDef *args = (SystemTaskArgsTypeDef *)argument;
  if (args == NULL) {
    printf("ERROR: System task args NULL\n");
    Error_Handler();
  }

  /* Store global pointer for API helpers */
  g_sys_ctx = args->system_context_access;

  if (g_sys_ctx == NULL || g_sys_ctx->mutex == NULL) {
    printf("ERROR: systemContextAccess not initialized or mutex NULL\n");
    Error_Handler();
  }

  if (args->system2_vp_queue == NULL) {
    printf("ERROR: system2_vp_queue NULL\n");
    Error_Handler();
  }

#if OS_TASKS_DEBUG
  printf("SystemTask running (heap=%lu)\n",
         (unsigned long)xPortGetFreeHeapSize());
#endif

  /* Initialize the state machine with task arguments */
  SystemSM_Init(args);

  /* Main control loop: execute state machine periodically */
  for (;;) {
    /* Run one iteration of the state machine */
    SystemSM_Run();

    /* Yield to prevent task starvation */
    osDelay(pdMS_TO_TICKS(100));
  }
}