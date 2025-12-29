#include "view_presenter_task.h"
#include "input_task.h"
#include "lvgl_port_display.h"
#include "view_presenter_router.h"
#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"

#define VIEW_DELAY_MS 10U

void StartViewPresenterTask(void *argument)
{
    const ViewPresenterTaskArgsTypeDef *args = (const ViewPresenterTaskArgsTypeDef *)argument;
    if (args == NULL)
    {
        Error_Handler();
    }

    osMessageQueueId_t input2vp_event_queue = args->input2vp_event_queue;
    if (input2vp_event_queue == NULL)
    {
        Error_Handler();
    }

    osMessageQueueId_t system2vp_event_queue = args->system2vp_event_queue;
    if (system2vp_event_queue == NULL)
    {
        Error_Handler();
    }

    /* Optional: pointer to shared system context for UI reading */
    SystemContextAccessTypeDef *sys_ctx = args->system_context_access;
    (void)sys_ctx; /* currently unused; provided for UI read access */

#if OS_TASKS_DEBUG
    printf("ViewPresenterTask running (heap=%lu)\n", (unsigned long)xPortGetFreeHeapSize());
#endif
    
    Router_Init(args->vp2system_event_queue, args->system_context_access, args->config_access, args->sensor_values_access);

    Input2VPEvent_t event;
    System2VPEventTypeDef sys_event;
    uint8_t init_complete = 0;

    printf("ViewPresenter task waiting for system init...\n");

    /* Wait for system initialization to complete */
    while (!init_complete)
    {
        if (osMessageQueueGet(system2vp_event_queue, &sys_event, NULL, osWaitForever) == osOK)
        {
            if (sys_event == EVT_SYS_INIT_END)
            {
                init_complete = 1;
                printf("ViewPresenter received EVT_SYS_INIT_END. Starting main loop...\n");
            }
        }
    }

    for(;;)
    {
        // Wait for event with timeout to allow periodic updates
        // Use the timeout to pace the loop when idle, but process immediately when busy
        osStatus_t queue_status = osMessageQueueGet(input2vp_event_queue, &event, NULL, pdMS_TO_TICKS(VIEW_DELAY_MS));
        if (queue_status == osOK) {
            printf("ViewPresenterTask: Received event type=%d\n", event.type);
            Router_HandleEvent(&event);
            
            // Drain remaining events in the queue without blocking
            while (osMessageQueueGet(input2vp_event_queue, &event, NULL, 0) == osOK) {
                printf("ViewPresenterTask: Received event (drained) type=%d\n", event.type);
                Router_HandleEvent(&event);
            }
        }

        // Always call OnTick without stopping rendering
        // This ensures continuous animation and updates
        Router_OnTick(osKernelGetTickCount());

        // Give other tasks (LVGL rendering, sensor, etc.) a chance to run.
        osDelay(pdMS_TO_TICKS(5U));
    }
}
