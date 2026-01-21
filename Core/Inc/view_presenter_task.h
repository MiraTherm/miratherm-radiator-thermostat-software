/**
 ******************************************************************************
 * @file           :  view_presenter_task.h
 * @brief          :  User interface presentation and rendering task
 *
 * @details        :  Manages the Model-View-Presenter (MVP) pattern for the
 *                    user interface. Processes input events, coordinates with
 *                    the view/presenter router, and maintains the display.
 *                    Synchronizes with system task for state transitions and
 *                    accesses shared sensor and configuration data.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#ifndef CORE_INC_VIEW_PRESENTER_TASK_H
#define CORE_INC_VIEW_PRESENTER_TASK_H

#include "input_task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def VP_TASK_STACK_SIZE
 * @brief Stack size in bytes for the view presenter task
 * @details Critical: insufficient stack will cause hard faults!
 *          Allocated from FreeRTOS heap during task creation
 */
#define VP_TASK_STACK_SIZE (1024U * 4U)

#include "sensor_task.h"
#include "storage_task.h"
#include "system_task.h"

/**
 * @typedef ViewPresenterTaskArgsTypeDef
 * @brief Arguments passed to StartViewPresenterTask
 * @details Contains all queue handles and shared data access pointers needed
 *          by the UI task for inter-task communication and rendering.
 * @see StartViewPresenterTask
 */
typedef struct {
  osMessageQueueId_t input2vp_event_queue;    /**< Input events from user (buttons, encoder) */
  osMessageQueueId_t vp2system_event_queue;   /**< UI sends events to system (user actions) */
  osMessageQueueId_t system2vp_event_queue;   /**< System sends events to UI (state changes) */
  SystemContextAccessTypeDef
      *system_context_access;                 /**< Pointer to system context for UI display */
  ConfigAccessTypeDef *config_access;         /**< Pointer to configuration data access */
  SensorValuesAccessTypeDef
      *sensor_values_access;                  /**< Pointer to sensor values for display */
} ViewPresenterTaskArgsTypeDef;

/**
 * @brief Start the user interface presentation task
 * @details Initializes the MVP router, waits for system initialization complete
 *          event, then enters main UI loop. Processes input events, manages
 *          route transitions based on system state, and performs periodic
 *          display updates including animations.
 * @param argument Pointer to ViewPresenterTaskArgsTypeDef containing event
 *                 queues and shared data access pointers. NULL argument causes
 *                 Error_Handler() to be called.
 * @return Does not return; runs as infinite FreeRTOS task
 * @see ViewPresenterTaskArgsTypeDef, Router_Init, Router_HandleEvent, Router_OnTick
 */
void StartViewPresenterTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEW_PRESENTER_TASK_H */
