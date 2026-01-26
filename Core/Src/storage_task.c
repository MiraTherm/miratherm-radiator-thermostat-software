/**
 ******************************************************************************
 * @file           :  storage_task.c
 * @brief          :  Implementation of Flash-based configuration storage and
 *                    management task
 *
 * @details        :  Manages EEPROM emulation in STM32WB55 Flash, handles
 *                    configuration read/write with checksum validation, and
 *                    provides thread-safe persistent storage operations.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#include "storage_task.h"

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "main.h"
#include "stm32wbxx_hal.h"
#include "task.h"
#include "task_debug.h"
#include "utils.h"

#include <string.h>

/* EEPROM Emulation in Flash: STM32WB55 has 512KB Flash, use last 4KB page */
#define EEPROM_START_ADDR (FLASH_BASE + 512 * 1024 - 4 * 1024)
#define EEPROM_SIZE 4096U
#define CONFIG_MAGIC_NUMBER 0xDEADBEEFU
#define CONFIG_VERSION 1U
#define STORAGE_EVENT_QUEUE_DEPTH 4U

/* Configuration storage block format: magic + version + config + checksum */
typedef struct {
  uint32_t magic;
  uint32_t version;
  ConfigTypeDef config;
  uint32_t checksum;
} StorageBlockTypeDef;

/* Thread-safe access to configuration and event queues */
static ConfigAccessTypeDef *s_config_access = NULL;
static osMessageQueueId_t s_event_queue = NULL;
static osMessageQueueId_t s_system2storage_queue = NULL;

/* Calculate simple checksum for config validation and corruption detection */
static uint32_t calculate_checksum(const ConfigTypeDef *config) {
  if (config == NULL)
    return 0;

  uint32_t checksum = 0;
  const uint8_t *data = (const uint8_t *)config;
  size_t size = sizeof(ConfigTypeDef);

  for (size_t i = 0; i < size; i++) {
    checksum += data[i];
    checksum = (checksum << 1) | (checksum >> 31); /* Rotate left for better distribution */
  }

  return checksum;
}

/* Read configuration from Flash with validation */
static bool read_config_from_flash(ConfigTypeDef *config) {
  if (config == NULL)
    return false;

  const StorageBlockTypeDef *block =
      (const StorageBlockTypeDef *)EEPROM_START_ADDR;

  /* Validate magic number and version */
  if (block->magic != CONFIG_MAGIC_NUMBER)
    return false;

  if (block->version != CONFIG_VERSION)
    return false;

  /* Validate checksum to detect corruption */
  uint32_t calculated_checksum = calculate_checksum(&block->config);
  if (calculated_checksum != block->checksum)
    return false;

  *config = block->config;
  return true;
}

/* Write configuration to Flash with sector erase */
static bool write_config_to_flash(const ConfigTypeDef *config) {
  if (config == NULL)
    return false;

  StorageBlockTypeDef block;
  block.magic = CONFIG_MAGIC_NUMBER;
  block.version = CONFIG_VERSION;
  block.config = *config;
  block.checksum = calculate_checksum(config);

  /* Unlock Flash for writing */
  if (HAL_FLASH_Unlock() != HAL_OK)
    return false;

  /* Erase the EEPROM sector (single 4KB page) */
  FLASH_EraseInitTypeDef erase_init = {0};
  erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  erase_init.Page = (EEPROM_START_ADDR - FLASH_BASE) / FLASH_PAGE_SIZE;
  erase_init.NbPages = 1;
  uint32_t page_error = 0;

  if (HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK) {
    HAL_FLASH_Lock();
    return false;
  }

  /* Program data as 64-bit words (STM32WB55 requires double-word writes) */
  uint8_t *src_bytes = (uint8_t *)&block;
  uint64_t *dst = (uint64_t *)EEPROM_START_ADDR;
  size_t total_bytes = sizeof(StorageBlockTypeDef);
  size_t words = (total_bytes + 7) / 8;

  for (size_t i = 0; i < words; i++) {
    uint64_t data = 0;
    /* Construct 64-bit word safely from source bytes */
    size_t copy_len = (total_bytes - i * 8);
    if (copy_len > 8)
      copy_len = 8;

    memcpy(&data, src_bytes + i * 8, copy_len);

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)(dst + i),
                          data) != HAL_OK) {
      HAL_FLASH_Lock();
      return false;
    }
  }

  /* Lock Flash after writing */
  HAL_FLASH_Lock();
  return true;
}

/* Post event to system via event queue */
static void StorageTask_PostEvent(Storage2SystemEventTypeDef event) {
  if (s_event_queue == NULL)
    return;

  Storage2SystemEventTypeDef evt_copy = event;
  osStatus_t status = osMessageQueuePut(s_event_queue, &evt_copy, 0U, 0U);
  if (status != osOK) {
    printf("StorageTask: Failed to post event (status=%d)\n", status);
  }
}

/* Main storage task: manages Flash persistence and configuration events */
void StartStorageTask(void *argument) {
  StorageTaskArgsTypeDef *args = (StorageTaskArgsTypeDef *)argument;
  osMessageQueueId_t event_queue = args->storage2system_event_queue;
  osMessageQueueId_t system2storage_queue = args->system2storage_event_queue;
  ConfigAccessTypeDef *config_access = args->config_access;

  /* Receive event queue and config access handles from main */
  if (event_queue != NULL) {
    s_event_queue = event_queue;
  }

  if (system2storage_queue != NULL) {
    s_system2storage_queue = system2storage_queue;
  }

  if (config_access != NULL) {
    s_config_access = config_access;
  }

  /* Verify both are available */
  if (s_event_queue == NULL) {
    Error_Handler();
  }

  if (s_config_access == NULL || s_config_access->mutex == NULL) {
    Error_Handler();
  }

  /* Load configuration from Flash or initialize defaults */
  ConfigTypeDef loaded_config = {.temperature_offset = 0.0f,
                                 .manual_target_temp = 20.0f};
  if (read_config_from_flash(&loaded_config)) {
    /* Store in shared config with mutex protection */
    if (osMutexAcquire(s_config_access->mutex, osWaitForever) == osOK) {
      s_config_access->data = loaded_config;
      osMutexRelease(s_config_access->mutex);
    }
    printf("StorageTask: Configuration loaded from Flash\n");
  } else {
    printf("StorageTask: No valid configuration in Flash, using defaults\n");
    /* Save default configuration to Flash */
    ConfigTypeDef default_config = {.temperature_offset = 0.0f,
                                    .manual_target_temp = 20.0f};
    Utils_LoadDefaultSchedule(&default_config.daily_schedule, 3);
    if (write_config_to_flash(&default_config)) {
      if (osMutexAcquire(s_config_access->mutex, osWaitForever) == osOK) {
        s_config_access->data = default_config;
        osMutexRelease(s_config_access->mutex);
      }
      printf("StorageTask: Default configuration saved to Flash\n");
    } else {
      printf("StorageTask: Failed to save default configuration\n");
    }
  }

  /* Signal that config loading is complete */
  StorageTask_PostEvent(EVT_CFG_LOAD_END);

#if OS_TASKS_DEBUG
  printf("StorageTask running (heap=%lu)\n",
         (unsigned long)xPortGetFreeHeapSize());
#endif

  /* Track last written config to detect changes */
  ConfigTypeDef last_written_config = s_config_access->data;

  /* Periodic monitoring loop: check every 2.5 seconds for config changes */
  for (;;) {
    System2StorageEventTypeDef sysEvt;
    osStatus_t status = osMessageQueueGet(s_system2storage_queue, &sysEvt, NULL,
                                          pdMS_TO_TICKS(2500U));

    /* Handle factory reset request */
    if (status == osOK) {
      if (sysEvt == EVT_CFG_RST_REQ) {
        printf("StorageTask: Factory Reset Requested\n");
        ConfigTypeDef default_config = {.temperature_offset = 0.0f,
                                        .manual_target_temp = 20.0f};
        Utils_LoadDefaultSchedule(&default_config.daily_schedule, 3);

        if (write_config_to_flash(&default_config)) {
          if (osMutexAcquire(s_config_access->mutex, osWaitForever) == osOK) {
            s_config_access->data = default_config;
            last_written_config = default_config; /* Prevent re-write */
            osMutexRelease(s_config_access->mutex);
          }
          printf("StorageTask: Factory Reset Complete\n");
          StorageTask_PostEvent(EVT_CFG_RST_END);
        }
      }
    }

    /* Check if config changed and write to Flash if needed */
    if (osMutexAcquire(s_config_access->mutex, osWaitForever) == osOK) {
      bool config_changed =
          (memcmp(&s_config_access->data, &last_written_config,
                  sizeof(ConfigTypeDef)) != 0);

      if (config_changed) {
        ConfigTypeDef current_data = s_config_access->data;
        osMutexRelease(s_config_access->mutex);

        /* Config changed, persist to Flash */
        if (write_config_to_flash(&current_data)) {
          if (osMutexAcquire(s_config_access->mutex, osWaitForever) == osOK) {
            last_written_config = s_config_access->data;
            osMutexRelease(s_config_access->mutex);
          }
          printf("StorageTask: Configuration saved to Flash\n");
        } else {
          printf("StorageTask: Failed to save configuration to Flash\n");
        }
      } else {
        osMutexRelease(s_config_access->mutex);
      }
    }
  }
}
