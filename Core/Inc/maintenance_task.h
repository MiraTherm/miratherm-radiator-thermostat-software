/**
 ******************************************************************************
 * @file           :  maintenance_task.h
 * @brief          :  Radiator valve maintenance task
 *
 * @details        :  Executes maintenance procedures on the radiator valve 
 *                    such as valve adaptation/calibration (currently mocked).
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#ifndef CORE_INC_MAINTENANCE_TASK_H
#define CORE_INC_MAINTENANCE_TASK_H

#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def MAINT_TASK_STACK_SIZE
 * @brief Stack size in bytes for the maintenance task
 * @details Allocated from FreeRTOS heap during task creation
 */
#define MAINT_TASK_STACK_SIZE (512U * 4U)

/**
 * @typedef System2MaintEventTypeDef
 * @brief System to Maintenance task command type
 * @details Commands requesting maintenance operations:
 *          - EVT_ADAPT_START: Initiate radiator adaptation/calibration
 */
typedef enum {
  EVT_ADAPT_START = 0  /**< Start radiator adaptation procedure */
} System2MaintEventTypeDef;

/**
 * @typedef MaintResultTypeDef
 * @brief Maintenance operation result enumeration
 * @details Result status for completed operations
 */
typedef enum {
  OK = 0,              /**< Operation succeeded */
  FAIL                 /**< Operation failed */
} MaintResultTypeDef;

/**
 * @typedef Maint2SystemEventTypeDef
 * @brief Maintenance to System task event type
 * @details Notifications of completed maintenance operations:
 *          - EVT_ADAPT_END: Radiator adaptation complete (check result field)
 */
typedef enum {
  EVT_ADAPT_END        /**< Radiator adaptation complete */
} Maint2SystemEventTypeDef;

/**
 * @typedef Maint2SystemEvent_t
 * @brief Complete maintenance result event with status
 * @details Contains event type and result code for operation status reporting
 * @see Maint2SystemEventTypeDef, MaintResultTypeDef
 */
typedef struct {
  Maint2SystemEventTypeDef type;  /**< Event type (EVT_ADAPT_END) */
  MaintResultTypeDef result;      /**< Operation result (OK/FAIL) */
} Maint2SystemEvent_t;

/**
 * @typedef MaintenanceTaskArgsTypeDef
 * @brief Arguments passed to StartMaintenanceTask
 * @details Contains event queue handles for inter-task communication
 * @see StartMaintenanceTask
 */
typedef struct {
  osMessageQueueId_t system2maint_event_queue;  /**< System -> Maintenance command queue */
  osMessageQueueId_t maint2system_event_queue;  /**< Maintenance -> System result queue */
} MaintenanceTaskArgsTypeDef;

/**
 * @brief Start the maintenance and adaptation task
 * @details Processes maintenance commands from the system task, performs
 *          long-running operations like radiator adaptation, and reports
 *          results back via event queue. (All operations are currently mocked.)
 * @param argument Pointer to MaintenanceTaskArgsTypeDef containing event queues.
 *                 NULL argument causes Error_Handler() to be called.
 * @return Does not return; runs as infinite FreeRTOS task
 * @see MaintenanceTaskArgsTypeDef, System2MaintEventTypeDef, Maint2SystemEvent_t
 */
void StartMaintenanceTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_MAINTENANCE_TASK_H */
