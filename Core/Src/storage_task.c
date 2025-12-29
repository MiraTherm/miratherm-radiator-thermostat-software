#include "storage_task.h"

#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "task_debug.h"
#include "stm32wbxx_hal.h"

#include <string.h>

/* EEPROM Emulation Layer Configuration */
/* Using end of Flash for EEPROM emulation: 
   STM32WB55 has 512KB Flash, we use the last page (4KB) */
#define EEPROM_START_ADDR (FLASH_BASE + 512 * 1024 - 4 * 1024)
#define EEPROM_SIZE 4096U
#define CONFIG_MAGIC_NUMBER 0xDEADBEEFU
#define CONFIG_VERSION 1U
#define STORAGE_EVENT_QUEUE_DEPTH 4U

typedef struct
{
  uint32_t magic;
  uint32_t version;
  ConfigTypeDef config;
  uint32_t checksum;
} StorageBlockTypeDef;

static ConfigAccessTypeDef *s_config_access = NULL;
static osMessageQueueId_t s_event_queue = NULL;

/**
 * Calculate a simple checksum for config validation
 */
static uint32_t calculate_checksum(const ConfigTypeDef *config)
{
  if (config == NULL)
    return 0;

  uint32_t checksum = 0;
  const uint8_t *data = (const uint8_t *)config;
  size_t size = sizeof(ConfigTypeDef);

  for (size_t i = 0; i < size; i++)
  {
    checksum += data[i];
    checksum = (checksum << 1) | (checksum >> 31);  /* Rotate left */
  }

  return checksum;
}

/**
 * Read configuration from Flash
 */
static bool read_config_from_flash(ConfigTypeDef *config)
{
  if (config == NULL)
    return false;

  const StorageBlockTypeDef *block = (const StorageBlockTypeDef *)EEPROM_START_ADDR;

  /* Validate magic number and version */
  if (block->magic != CONFIG_MAGIC_NUMBER)
    return false;

  if (block->version != CONFIG_VERSION)
    return false;

  /* Validate checksum */
  uint32_t calculated_checksum = calculate_checksum(&block->config);
  if (calculated_checksum != block->checksum)
    return false;

  *config = block->config;
  return true;
}

/**
 * Write configuration to Flash (with sector erase)
 */
static bool write_config_to_flash(const ConfigTypeDef *config)
{
  if (config == NULL)
    return false;

  StorageBlockTypeDef block;
  block.magic = CONFIG_MAGIC_NUMBER;
  block.version = CONFIG_VERSION;
  block.config = *config;
  block.checksum = calculate_checksum(config);

  /* Unlock Flash */
  if (HAL_FLASH_Unlock() != HAL_OK)
    return false;

  /* Erase the EEPROM sector */
  FLASH_EraseInitTypeDef erase_init = {0};
  erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  erase_init.Page = (EEPROM_START_ADDR - FLASH_BASE) / FLASH_PAGE_SIZE;
  erase_init.NbPages = 1;
  uint32_t page_error = 0;

  if (HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK)
  {
    HAL_FLASH_Lock();
    return false;
  }

  /* Program the new data (64-bit words) */
  /* Ensure we write all bytes, padding with 0 if necessary */
  uint8_t *src_bytes = (uint8_t *)&block;
  uint64_t *dst = (uint64_t *)EEPROM_START_ADDR;
  size_t total_bytes = sizeof(StorageBlockTypeDef);
  size_t words = (total_bytes + 7) / 8;

  for (size_t i = 0; i < words; i++)
  {
    uint64_t data = 0;
    /* Construct 64-bit word safely */
    size_t copy_len = (total_bytes - i * 8);
    if (copy_len > 8) copy_len = 8;
    
    memcpy(&data, src_bytes + i * 8, copy_len);

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)(dst + i), data) != HAL_OK)
    {
      HAL_FLASH_Lock();
      return false;
    }
  }

  /* Lock Flash */
  HAL_FLASH_Lock();
  return true;
}

static void StorageTask_PostEvent(Storage2SystemEventTypeDef event)
{
  if (s_event_queue == NULL)
    return;

  Storage2SystemEventTypeDef evt_copy = event;
  osStatus_t status = osMessageQueuePut(s_event_queue, &evt_copy, 0U, 0U);
  if (status != osOK)
  {
    printf("StorageTask: Failed to post event (status=%d)\n", status);
  }
}

bool StorageTask_TryGetEvent(Storage2SystemEventTypeDef *event, uint32_t timeout_ticks)
{
  if ((event == NULL) || (s_event_queue == NULL))
    return false;

  return (osMessageQueueGet(s_event_queue, event, NULL, timeout_ticks) == osOK);
}

void StartStorageTask(void *argument)
{
  StorageTaskArgsTypeDef *args = (StorageTaskArgsTypeDef *)argument;
  osMessageQueueId_t event_queue = args->storage2system_event_queue;
  ConfigAccessTypeDef *config_access = args->config_access;

  /* Receive event queue and config access handles from main */
  if (event_queue != NULL)
  {
    s_event_queue = event_queue;
  }

  if (config_access != NULL)
  {
    s_config_access = config_access;
  }

  /* Verify both are available */
  if (s_event_queue == NULL)
  {
    Error_Handler();
  }

  if (s_config_access == NULL || s_config_access->mutex == NULL)
  {
    Error_Handler();
  }

  /* Attempt to load configuration from Flash */
  ConfigTypeDef loaded_config = {.TemperatureOffsetC = 0.0f};
  if (read_config_from_flash(&loaded_config))
  {
    /* Store in shared config with mutex protection */
    if (osMutexAcquire(s_config_access->mutex, osWaitForever) == osOK)
    {
      s_config_access->data = loaded_config;
      osMutexRelease(s_config_access->mutex);
    }
    printf("StorageTask: Configuration loaded from Flash\n");
  }
  else
  {
    printf("StorageTask: No valid configuration in Flash, using defaults\n");
    /* Save default configuration to Flash */
    ConfigTypeDef default_config = {.TemperatureOffsetC = 0.0f};
    if (write_config_to_flash(&default_config))
    {
      printf("StorageTask: Default configuration saved to Flash\n");
    }
    else
    {
      printf("StorageTask: Failed to save default configuration\n");
    }
  }

  /* Signal that config loading is complete */
  StorageTask_PostEvent(EVT_CFG_LOAD_END);

#if OS_TASKS_DEBUG
  printf("StorageTask running (heap=%lu)\n", (unsigned long)xPortGetFreeHeapSize());
#endif

  /* Track last written config to detect changes */
  ConfigTypeDef last_written_config = s_config_access->data;

  /* Periodic write task: check every 2.5 seconds if config changed */
  for (;;)
  {
    osDelay(pdMS_TO_TICKS(2500U));  /* Check every 2.5 seconds */

    if (osMutexAcquire(s_config_access->mutex, osWaitForever) == osOK)
    {
      /* Check if config changed by comparing with last written version */
      bool config_changed = (memcmp(&s_config_access->data, &last_written_config, sizeof(ConfigTypeDef)) != 0);

      if (config_changed)
      {
        ConfigTypeDef current_data = s_config_access->data;
        osMutexRelease(s_config_access->mutex);

        /* Config changed, write to Flash */
        if (write_config_to_flash(&current_data))
        {
          if (osMutexAcquire(s_config_access->mutex, osWaitForever) == osOK)
          {
            last_written_config = s_config_access->data;
            osMutexRelease(s_config_access->mutex);
          }
          printf("StorageTask: Configuration saved to Flash\n");
        }
        else
        {
          printf("StorageTask: Failed to save configuration to Flash\n");
        }
      }
      else
      {
        osMutexRelease(s_config_access->mutex);
      }
    }
  }
}
