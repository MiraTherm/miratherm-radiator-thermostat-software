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

    osMessageQueueId_t input2vp_queue = args->input2vp_event_queue;
    if (input2vp_queue == NULL)
    {
        Error_Handler();
    }

#if OS_TASKS_DEBUG
    printf("ViewPresenterTask running (heap=%lu)\n", (unsigned long)xPortGetFreeHeapSize());
#endif
    
    Router_Init();

    Input2VPEvent_t event;

    printf("ViewPresenter task init OK. Running loop...\n");

    for(;;)
    {
        // Wait for event with timeout to allow periodic updates
        // Use the timeout to pace the loop when idle, but process immediately when busy
        if (osMessageQueueGet(input2vp_queue, &event, NULL, pdMS_TO_TICKS(VIEW_DELAY_MS)) == osOK) {
            Router_HandleEvent(&event);
            
            // Drain remaining events in the queue without blocking
            while (osMessageQueueGet(input2vp_queue, &event, NULL, 0) == osOK) {
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
