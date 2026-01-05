#include "home_presenter.h"
#include "view_presenter_router.h"
#include "main.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

extern RTC_HandleTypeDef hrtc;

struct HomePresenter
{
    HomeView_t *view;
    SystemContextAccessTypeDef *system_context;
    ConfigAccessTypeDef *config_access;
    SensorValuesAccessTypeDef *sensor_values_access;
};

HomePresenter_t* HomePresenter_Init(HomeView_t *view, SystemContextAccessTypeDef *system_context, ConfigAccessTypeDef *config_access, SensorValuesAccessTypeDef *sensor_values_access)
{
    if (!view || !system_context || !config_access || !sensor_values_access)
        return NULL;

    HomePresenter_t *presenter = (HomePresenter_t *)malloc(sizeof(HomePresenter_t));
    if (!presenter)
        return NULL;

    presenter->view = view;
    presenter->system_context = system_context;
    presenter->config_access = config_access;
    presenter->sensor_values_access = sensor_values_access;

    return presenter;
}

void HomePresenter_Deinit(HomePresenter_t *presenter)
{
    if (presenter)
    {
        free(presenter);
    }
}

void HomePresenter_HandleEvent(HomePresenter_t *presenter, const Input2VPEvent_t *event)
{
    if (!presenter || !event)
        return;

    /* Handle rotary encoder movement */
    if (event->type == EVT_CTRL_WHEEL_DELTA)
    {
        if (!presenter->system_context)
            return;

        if (osMutexAcquire(presenter->system_context->mutex, 10) == osOK)
        {
            /* Use current temporary override if set, otherwise start from scheduled target */
            float current_temp;
            if (presenter->system_context->data.temporary_target_temp != 0)
            {
                current_temp = presenter->system_context->data.temporary_target_temp;
            }
            else
            {
                current_temp = presenter->system_context->data.target_temp;
            }
            
            uint16_t current_index = Utils_TempToIndex(current_temp);

            /* Adjust index by delta (each encoder click = 1 step) */
            int16_t new_index = (int16_t)current_index + event->delta;
            if (new_index < 0) new_index = 0;
            if (new_index > 51) new_index = 51;

            float new_temp = Utils_IndexToTemp((uint16_t)new_index);
            presenter->system_context->data.temporary_target_temp = new_temp;

            printf("Home: Rotary encoder delta=%d, new temp override=%.1f°C\n", event->delta, new_temp);

            osMutexRelease(presenter->system_context->mutex);
        }

        return;
    }

    /* Handle button presses */
    if (event->button_action == BUTTON_ACTION_PRESSED)
    {
        switch (event->type)
        {
            case EVT_MODE_BTN:
                /* TODO: Handle Mode button */
                printf("Home: Mode button pressed\n");
                break;
            case EVT_CENTRAL_BTN:
                /* TODO: Handle Boost button */
                printf("Home: Boost button pressed\n");
                break;
            case EVT_MENU_BTN:
                /* Handle Menu button */
                printf("Home: Menu button pressed\n");
                Router_GoToRoute(ROUTE_MENU);
                break;
            default:
                break;
        }
    }
}

void HomePresenter_Run(HomePresenter_t *presenter, uint32_t current_tick)
{
    if (!presenter || !presenter->view)
        return;

    HomeViewModel_t model = {0};

    /* Get Time */
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    model.hour = sTime.Hours;
    model.minute = sTime.Minutes;

    /* Get Sensor Values */
    if (osMutexAcquire(presenter->sensor_values_access->mutex, 10) == osOK)
    {
        model.current_temp = presenter->sensor_values_access->data.CurrentTemp;
        model.battery_percentage = presenter->sensor_values_access->data.SoC;
        osMutexRelease(presenter->sensor_values_access->mutex);
    }

    /* Get Target Temperature from System State (calculated in RUNNING state) */
    if (osMutexAcquire(presenter->system_context->mutex, 10) == osOK)
    {
        model.target_temp = presenter->system_context->data.target_temp;
        
        /* Use temporary override if set, otherwise use scheduled target */
        if (presenter->system_context->data.temporary_target_temp != 0)
        {
            model.target_temp = presenter->system_context->data.temporary_target_temp;
        }
        
        model.slot_end_hour = presenter->system_context->data.slot_end_hour;
        model.slot_end_minute = presenter->system_context->data.slot_end_minute;
        
        /* Determine if displaying OFF or ON mode */
        model.is_off_mode = (model.target_temp <= 4.5f);
        model.is_on_mode = (model.target_temp >= 30.0f);
        
        osMutexRelease(presenter->system_context->mutex);
    }

    HomeView_Render(presenter->view, &model);
}
