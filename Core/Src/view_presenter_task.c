/**
 ******************************************************************************
 * @file           :  view_presenter_task.c
 * @brief          :  Implementation of user interface presentation task
 *
 * @details        :  Manages the main UI loop, input event processing,
 *                    router coordination, and periodic display updates.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#include "view_presenter_task.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "input_task.h"
#include "lvgl_port_display.h"
#include "main.h"
#include "task.h"
#include "view_presenter_router.h"

/* Display update interval in milliseconds */
#define VIEW_DELAY_MS 10U

/* Main UI presentation task: handles input, routing, and display */
void StartViewPresenterTask(void *argument) {
  const ViewPresenterTaskArgsTypeDef *args =
      (const ViewPresenterTaskArgsTypeDef *)argument;
  if (args == NULL) {
    Error_Handler();
  }

  osMessageQueueId_t input2vp_event_queue = args->input2vp_event_queue;
  if (input2vp_event_queue == NULL) {
    Error_Handler();
  }

  osMessageQueueId_t system2vp_event_queue = args->system2vp_event_queue;
  if (system2vp_event_queue == NULL) {
    Error_Handler();
  }

  /* Optional: pointer to shared system context for UI state reading */
  SystemModel_t *sys_ctx = args->system_model;
  (void)sys_ctx; /* Currently unused; provided for future UI read access */

#if OS_TASKS_DEBUG
  printf("ViewPresenterTask running (heap=%lu)\n",
         (unsigned long)xPortGetFreeHeapSize());
#endif

  /* Initialize MVP router with all queues and data access */
  Router_Init(args->vp2system_event_queue, args->system_model,
              args->config_model, args->sensor_model);

  Input2VPEvent_t event;
  System2VPEventTypeDef sys_event;
  uint8_t init_complete = 0;

  printf("ViewPresenter task waiting for system init...\n");

  /* Wait for system initialization to complete before rendering */
  while (!init_complete) {
    if (osMessageQueueGet(system2vp_event_queue, &sys_event, NULL,
                          osWaitForever) == osOK) {
      if (sys_event == EVT_SYS_INIT_END) {
        init_complete = 1;
        printf(
            "ViewPresenter received EVT_SYS_INIT_END. Starting main loop...\n");
      }
    }
  }

  /* Main UI loop: process input and render display */
  for (;;) {
    /* Wait for input event with timeout to allow periodic updates */
    osStatus_t queue_status = osMessageQueueGet(
        input2vp_event_queue, &event, NULL, pdMS_TO_TICKS(VIEW_DELAY_MS));
    if (queue_status == osOK) {
#if VIEW_PRESENTER_TASK_DEBUG_PRINTING
      printf("ViewPresenterTask: Received event type=%d\n", event.type);
#endif
      /* Process single input event */
      Router_HandleEvent(&event);

      /* Drain remaining queued events without blocking */
      while (osMessageQueueGet(input2vp_event_queue, &event, NULL, 0) == osOK) {
#if VIEW_PRESENTER_TASK_DEBUG_PRINTING
        printf("ViewPresenterTask: Received event (drained) type=%d\n",
               event.type);
#endif
        Router_HandleEvent(&event);
      }
    }

    /* Periodic display update: animations and state changes */
    /* This ensures continuous rendering even with no input */
    Router_OnTick(osKernelGetTickCount());

    /* Yield to allow other tasks (LVGL, sensor, storage) to run */
    osDelay(pdMS_TO_TICKS(5U));
  }
}
