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
    (void)argument;

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
        if (InputTask_TryGetVPEvent(&event, pdMS_TO_TICKS(VIEW_DELAY_MS))) {
            Router_HandleEvent(&event);
            
            // Drain remaining events in the queue without blocking
            while (InputTask_TryGetVPEvent(&event, 0)) {
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
