#include "home_presenter.h"
#include "main.h"
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
                /* TODO: Handle Menu button */
                printf("Home: Menu button pressed\n");
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

    /* Get Config Values (Target Temp & Schedule) */
    if (osMutexAcquire(presenter->config_access->mutex, 10) == osOK)
    {
        /* Determine current target temp based on schedule or manual override (if implemented) */
        /* For now, assume we follow schedule. */
        /* Find current time slot */
        ConfigTypeDef *cfg = &presenter->config_access->data;
        float target_temp = 20.0f; /* Default */
        uint8_t end_h = 0, end_m = 0;

        /* Simple linear search for slot containing current time */
        /* Assuming slots are sorted and non-overlapping for simplicity, or just find first match */
        /* Note: Real implementation might need more robust schedule logic */
        
        bool found = false;
        for (int i = 0; i < cfg->DailySchedule.NumTimeSlots; i++)
        {
            TimeSlotTypeDef *slot = &cfg->DailySchedule.TimeSlots[i];
            
            int current_mins = model.hour * 60 + model.minute;
            int start_mins = slot->StartHour * 60 + slot->StartMinute;
            int end_mins = slot->EndHour * 60 + slot->EndMinute;

            if (current_mins >= start_mins && current_mins < end_mins)
            {
                target_temp = slot->Temperature;
                end_h = slot->EndHour;
                end_m = slot->EndMinute;
                found = true;
                break;
            }
        }

        if (!found)
        {
            /* Fallback if no slot matches (e.g. gap in schedule) */
            target_temp = 18.0f; /* Eco temp? */
            end_h = 0;
            end_m = 0;
        }

        model.target_temp = target_temp;
        model.slot_end_hour = end_h;
        model.slot_end_minute = end_m;

        osMutexRelease(presenter->config_access->mutex);
    }

    HomeView_Render(presenter->view, &model);
}
