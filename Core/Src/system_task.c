#include "system_task.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "maintenance_task.h"
#include "storage_task.h"
#include "system_state_machine.h"
#include <stdio.h>

/* Global pointer to system context for helper APIs */
static SystemContextAccessTypeDef *g_sys_ctx = NULL;

/* The state machine implementation */
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

  /* Initialize the State Machine */
  SystemSM_Init(args);

  for (;;) {
    /* Run the State Machine */
    SystemSM_Run();

    /* Yield/Delay to prevent starvation */
    osDelay(pdMS_TO_TICKS(100));
  }
}