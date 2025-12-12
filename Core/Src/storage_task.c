#include "storage_task.h"

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

static ConfigTypeDef s_config = {
  .TemperatureOffsetC = 0.0f
};

static osMutexId_t s_config_mutex = NULL;
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
  uint64_t *src = (uint64_t *)&block;
  uint64_t *dst = (uint64_t *)EEPROM_START_ADDR;
  size_t words = sizeof(StorageBlockTypeDef) / sizeof(uint64_t);

  for (size_t i = 0; i < words; i++)
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)(dst + i), src[i]) != HAL_OK)
    {
      HAL_FLASH_Lock();
      return false;
    }
  }

  /* Lock Flash */
  HAL_FLASH_Lock();
  return true;
}

bool StorageTask_CopyConfig(ConfigTypeDef *dest)
{
  if (dest == NULL || s_config_mutex == NULL)
    return false;

  if (osMutexAcquire(s_config_mutex, osWaitForever) != osOK)
    return false;

  *dest = s_config;
  osMutexRelease(s_config_mutex);
  return true;
}

bool StorageTask_SetConfig(const ConfigTypeDef *src)
{
  if (src == NULL || s_config_mutex == NULL)
    return false;

  if (osMutexAcquire(s_config_mutex, osWaitForever) != osOK)
    return false;

  /* Write to Flash */
  bool flash_ok = write_config_to_flash(src);

  if (flash_ok)
  {
    /* Update RAM copy only if Flash write succeeded */
    s_config = *src;
  }

  osMutexRelease(s_config_mutex);
  return flash_ok;
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

  /* Receive event queue handle from main */
  if (event_queue != NULL)
  {
    s_event_queue = event_queue;
  }

  /* Create mutex for config protection */
  const osMutexAttr_t mutex_attr = {
    .name = "Config",
    .attr_bits = osMutexPrioInherit,
  };
  s_config_mutex = osMutexNew(&mutex_attr);
  if (s_config_mutex == NULL)
  {
    Error_Handler();
  }

  /* Verify event queue is available */
  if (s_event_queue == NULL)
  {
    Error_Handler();
  }

  /* Attempt to load configuration from Flash */
  ConfigTypeDef loaded_config = {.TemperatureOffsetC = 0.0f};
  if (read_config_from_flash(&loaded_config))
  {
    s_config = loaded_config;
    printf("StorageTask: Configuration loaded from Flash\n");
  }
  else
  {
    printf("StorageTask: No valid configuration in Flash, using defaults\n");
    /* Save default configuration to Flash */
    if (write_config_to_flash(&s_config))
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

  /* StorageTask mainly runs on-demand via SetConfig/CopyConfig calls.
     Keep task alive but mostly idle. */
  for (;;)
  {
    osDelay(pdMS_TO_TICKS(10000U));  /* Check every 10 seconds */
  }
}
