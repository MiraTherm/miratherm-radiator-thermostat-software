/**
 ******************************************************************************
 * @file           :  system_task.h
 * @brief          :  Main system control and state machine management task
 *
 * @details        :  Orchestrates overall device operation through a finite
 *                    state machine. Manages transitions between initialization,
 *                    configuration, adaptation, and running states. Handles
 *                    mode selection (AUTO/MANUAL/BOOST), boost timeout
 *                    management, and schedule-based temperature control.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#ifndef CORE_INC_SYSTEM_TASK_H
#define CORE_INC_SYSTEM_TASK_H

#include "cmsis_os2.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def SYSTEM_TASK_STACK_SIZE
 * @brief Stack size in bytes for the system control task
 * @details Allocated from FreeRTOS heap during task creation
 */
#define SYSTEM_TASK_STACK_SIZE (512U * 4U)

/**
 * @typedef SystemState_t
 * @brief System operational state enumeration
 * @details Defines all possible states in the system lifecycle:
 *          - STATE_INIT: Initial startup, waiting for config load
 *          - STATE_COD: Configuration on device (date/time and daily heating schedule) via UI
 *          - STATE_NOT_INST: Not installed (waiting for user to request adaptation)
 *          - STATE_ADAPT: Adaptation in progress (measuring radiator characteristics)
 *          - STATE_ADAPT_FAIL: Adaptation failed (user can retry)
 *          - STATE_RUNNING: Normal operation (heating control active)
 *          - STATE_FACTORY_RST: Factory reset in progress
 *          - STATE_MAINT: Maintenance mode (not implemented)
 */
typedef enum {
  STATE_INIT = 0,           /**< Startup initialization */
  STATE_COD,                /**< Configuration on device (date/time and schedule) */
  STATE_NOT_INST,           /**< Not installed - awaiting adaptation request */
  STATE_ADAPT,              /**< Adaptation in progress */
  STATE_ADAPT_FAIL,         /**< Adaptation failed */
  STATE_RUNNING,            /**< Running - normal operation */
  STATE_FACTORY_RST,        /**< Factory reset in progress */
  STATE_MAINT               /**< Maintenance mode (not implemented) */
} SystemState_t;

/**
 * @typedef SystemMode_t
 * @brief Operating mode enumeration
 * @details Defines heating control modes:
 *          - MODE_AUTO: Follow daily schedule from configuration
 *          - MODE_MANUAL: Use fixed manual target temperature
 *          - MODE_BOOST: Maximum heating for specified duration (300 seconds)
 */
typedef enum {
  MODE_AUTO = 0,            /**< Schedule-based automatic mode */
  MODE_MANUAL = 1,          /**< Fixed temperature manual mode */
  MODE_BOOST = 2            /**< Opens valve at 80% for 300 seconds*/
} SystemMode_t;

/**
 * @typedef AdaptResult_t
 * @brief Radiator adaptation result enumeration
 * @details Tracks adaptation result for display and retry logic
 */
typedef enum {
  ADAPT_RESULT_UNKNOWN = -1, /**< Adaptation not yet attempted */
  ADAPT_RESULT_OK = 0,       /**< Adaptation succeeded */
  ADAPT_RESULT_FAIL = 1      /**< Adaptation failed */
} AdaptResult_t;

/**
 * @typedef SystemData_t
 * @brief Core system control state and parameters
 * @details Contains current operational state, mode, target temperature,
 *          schedule information, and adaptation results. Protected by mutex
 *          for thread-safe access from multiple tasks.
 * @see SystemModel_t
 */
typedef struct {
  SystemState_t state;              /**< Current system state */
  SystemMode_t mode;                /**< Current operating mode (AUTO/MANUAL/BOOST) */
  SystemMode_t mode_before_boost;   /**< Previous mode before BOOST was activated */
  uint32_t boost_begin_time;        /**< Tick count when BOOST mode started */
  AdaptResult_t adapt_result;       /**< Result of last adaptation attempt */
  float target_temp;                /**< Target temperature in °C */
  uint8_t slot_end_hour;            /**< Current schedule slot end time (hour) */
  uint8_t slot_end_minute;          /**< Current schedule slot end time (minute) */
  float temporary_target_temp;      /**< Temporary override temperature (0=none, rotary encoder set) */
} SystemData_t;

/**
 * @typedef SystemModel_t
 * @brief Thread-safe access wrapper for system context
 * @details Provides osMutex-protected access to SystemData_t. Must be
 *          acquired before reading or modifying system state to ensure
 *          consistency across concurrent FreeRTOS tasks.
 * @see StartSystemTask
 */
typedef struct {
  osMutexId_t mutex;    /**< CMSIS-RTOS2 mutex for thread-safe access */
  SystemData_t data;    /**< System context data (protected by mutex) */
} SystemModel_t;

/**
 * @typedef VP2SystemEventTypeDef
 * @brief ViewPresenter to System event type
 * @details User interface events requesting state transitions:
 *          - EVT_COD_END: Configuration on device (date/time and schedule) complete
 *          - EVT_INST_REQ: Initiate radiator adaptation
 *          - EVT_ADAPT_RST_REQ: Retry adaptation after failure
 *          - EVT_FACTORY_RST_REQ: Perform factory reset
 */
typedef enum {
  EVT_NO_EVENT = 0,         /**< No event */
  EVT_COD_END,              /**< Configuration on device (date/time and schedule) complete */
  EVT_INST_REQ,             /**< Start adaptation request */
  EVT_ADAPT_RST_REQ,        /**< Retry adaptation request */
  EVT_FACTORY_RST_REQ       /**< Factory reset request */
} VP2SystemEventTypeDef;

/**
 * @typedef System2VPEventTypeDef
 * @brief System to ViewPresenter event type
 * @details System notifications for UI state updates:
 *          - EVT_SYS_INIT_END: Initialization complete, UI can begin rendering
 */
typedef enum {
  EVT_SYS_INIT_END = 0      /**< System initialization complete */
} System2VPEventTypeDef;

/* Forward declaration */
typedef struct ConfigModel_t ConfigModel_t;

/**
 * @typedef SystemTaskArgsTypeDef
 * @brief Arguments passed to StartSystemTask
 * @details Contains all queue handles and shared data access pointers needed
 *          by the system task for inter-task communication.
 * @see StartSystemTask
 */
typedef struct {
  osMessageQueueId_t vp2system_event_queue;       /**< ViewPresenter -> System event queue */
  osMessageQueueId_t system2vp_event_queue;       /**< System -> ViewPresenter event queue */
  osMessageQueueId_t system2maint_event_queue;    /**< System -> Maintenance command queue */
  osMessageQueueId_t maint2system_event_queue;    /**< Maintenance -> System result queue */
  osMessageQueueId_t system2storage_event_queue;  /**< System -> Storage command queue */
  SystemModel_t
      *system_model;                              /**< Pointer to system state context */
  ConfigModel_t
      *config_model;                              /**< Pointer to configuration/schedule data */
} SystemTaskArgsTypeDef;

/**
 * @brief Start the main system control task
 * @details Initializes the system state machine, enters the main control loop,
 *          and manages all state transitions. Processes events from ViewPresenter,
 *          Maintenance, and Storage tasks. Updates shared system context with
 *          current state and target temperature.
 * @param argument Pointer to SystemTaskArgsTypeDef containing event queues
 *                 and shared data access pointers. NULL argument causes
 *                 Error_Handler() to be called.
 * @return Does not return; runs as infinite FreeRTOS task
 * @note Called from app_freertos.c during RTOS initialization
 * @see SystemTaskArgsTypeDef, SystemModel_t, SystemState_t
 */
void StartSystemTask(void *argument);

/**
 * @brief Query state machine current state internally
 * @details Used for state transition logic within system_state_machine.c.
 *          Returns the current state without mutex acquisition.
 * @return Current system state
 * @note Internal use only; prefer System_GetState() for external access
 * @see System_GetState, SystemState_t
 */
SystemState_t SystemSM_GetCurrentState(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_SYSTEM_TASK_H */
