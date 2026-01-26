/**
 ******************************************************************************
 * @file           :  storage_task.h
 * @brief          :  Flash-based configuration storage and management task
 *
 * @details        :  Manages persistent storage of device configuration using
 *                    STM32WB55 Flash EEPROM emulation. Provides configuration
 *                    read/write operations with checksum validation, factory
 *                    reset functionality, and thread-safe access via mutex.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#ifndef CORE_STORAGE_TASK_H
#define CORE_STORAGE_TASK_H

#include <stdbool.h>
#include <stdint.h>

#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef Storage2SystemEventTypeDef
 * @brief Event type for notifications from storage task to system
 * @details Events signaling completion of storage operations
 */
typedef enum {
  EVT_CFG_LOAD_END = 0,  /**< Configuration load from Flash complete */
  EVT_CFG_RST_END        /**< Factory reset complete */
} Storage2SystemEventTypeDef;

/**
 * @typedef System2StorageEventTypeDef
 * @brief Event type for requests from system to storage task
 * @details Events requesting storage operations
 */
typedef enum { EVT_CFG_RST_REQ = 0 /**< Factory reset request */ } System2StorageEventTypeDef;

/**
 * @typedef TimeSlotTypeDef
 * @brief Daily schedule time slot configuration
 * @details Defines a time window with associated target temperature
 */
typedef struct {
  uint8_t start_hour;    /**< Slot start hour (0-23) */
  uint8_t start_minute;  /**< Slot start minute (0-59) */
  uint8_t end_hour;      /**< Slot end hour (0-23) */
  uint8_t end_minute;    /**< Slot end minute (0-59) */
  float temperature;    /**< Target temperature in °C for this slot */
} TimeSlotTypeDef;

/**
 * @typedef DailyScheduleTypeDef
 * @brief Daily heating schedule with multiple time slots
 * @details Contains up to 5 configurable time slots per day
 */
typedef struct {
  uint8_t num_time_slots;           /**< Number of active time slots (1-5) */
  TimeSlotTypeDef time_slots[5];   /**< Array of time slot configurations */
} DailyScheduleTypeDef;

/**
 * @typedef ConfigTypeDef
 * @brief Device configuration parameters stored in Flash
 * @details All user-configurable settings persisted across power cycles.
 *          Temperature offset allows sensor calibration compensation.
 *          Manual target temperature used in non-scheduled operation.
 * @see DailyScheduleTypeDef
 */
typedef struct {
  float temperature_offset;        /**< Temperature sensor calibration offset in °C */
  DailyScheduleTypeDef daily_schedule; /**< Daily heating schedule */
  float manual_target_temp;          /**< Manual mode target temperature in °C */
} ConfigTypeDef;

/**
 * @typedef ConfigAccessTypeDef
 * @brief Thread-safe access wrapper for device configuration
 * @details Provides osMutex-protected access to ConfigTypeDef. Must be
 *          acquired before reading or writing configuration to ensure
 *          consistency across concurrent FreeRTOS tasks.
 * @see StartStorageTask
 */
typedef struct ConfigAccessTypeDef {
  osMutexId_t mutex;    /**< CMSIS-RTOS2 mutex for thread-safe access */
  ConfigTypeDef data;   /**< Configuration data (protected by mutex) */
} ConfigAccessTypeDef;

/**
 * @brief Start the Flash storage management task
 * @details Initializes configuration storage, loads settings from Flash,
 *          establishes event queue communication channels, and enters main
 *          monitoring loop. Periodically checks for configuration changes
 *          and writes updates to Flash with checksum validation.
 * @param argument Pointer to StorageTaskArgsTypeDef containing event queues
 *                 and config access pointer. NULL argument causes
 *                 Error_Handler() to be called.
 * @return Does not return; runs as infinite FreeRTOS task
 * @see StorageTaskArgsTypeDef, ConfigAccessTypeDef, Storage2SystemEventTypeDef
 */
void StartStorageTask(void *argument);

/**
 * @brief Attempt to retrieve a storage event with timeout
 * @details Non-blocking or blocking retrieval of events from storage task
 *          event queue. Returns immediately with event if available, otherwise
 *          waits up to timeout_ticks.
 * @param event Pointer to Storage2SystemEventTypeDef to receive event data
 * @param timeout_ticks Maximum FreeRTOS ticks to wait (osWaitForever for
 *                       infinite wait, 0 for non-blocking)
 * @return true if event retrieved successfully, false if timeout or null pointer
 * @note Thread-safe; event queue is managed by storage task
 * @see Storage2SystemEventTypeDef, EVT_CFG_LOAD_END, EVT_CFG_RST_END
 */
bool StorageTask_TryGetEvent(Storage2SystemEventTypeDef *event,
                             uint32_t timeout_ticks);

/**
 * @def STORAGE_TASK_STACK_SIZE
 * @brief Stack size in bytes for the storage management task
 * @details Allocated from FreeRTOS heap during task creation
 */
#define STORAGE_TASK_STACK_SIZE (512U * 4U)

#ifdef __cplusplus
}
#endif

#endif /* CORE_STORAGE_TASK_H */
