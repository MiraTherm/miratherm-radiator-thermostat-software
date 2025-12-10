#include "view_presenter_task.h"
#include "input_task.h"
#include "lvgl_port_display.h"
#include "view_presenter_router.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"

void StartViewPresenterTask(void *argument)
{
    (void)argument;
    
    stop_rendering();
    Router_Init();
    start_rendering();

    Input2VPEvent_t event;

    for(;;)
    {
        // Wait for event with timeout to allow periodic updates
        if (InputTask_TryGetVPEvent(&event, pdMS_TO_TICKS(10U))) {
            stop_rendering();
            Router_HandleEvent(&event);
            start_rendering();
        }

        uint32_t current_tick = osKernelGetTickCount();
        // Handle tick wrap-around if necessary, though osKernelGetTickCount usually handles it well enough for simple deltas
        // Assuming 1 tick = 1 ms for simplicity, or convert. 
        // FreeRTOS config usually has 1000Hz tick.
        
        stop_rendering();
        Router_OnTick(current_tick);
        start_rendering();

        // Give other tasks (LVGL rendering, sensor, etc.) a chance to run.
        osDelay(50);
    }
}
