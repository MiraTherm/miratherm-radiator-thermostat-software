#include "set_time_presenter.h"
#include "set_time_view.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct SetTimePresenter
{
    SetTimeView_t *view;
    SetTime_ViewModelData_t data;
    bool is_complete;
    
    uint8_t time_hour_index;
    uint8_t time_minute_index;
} SetTimePresenter_t;

static const uint8_t HOURS_COUNT = 24;
static const uint8_t MINUTES_COUNT = 60;
static const uint8_t DEFAULT_HOUR = 12;
static const uint8_t DEFAULT_MINUTE = 0;

SetTimePresenter_t* SetTimePresenter_Init(SetTimeView_t *view)
{
    SetTimePresenter_t *presenter = (SetTimePresenter_t *)malloc(sizeof(SetTimePresenter_t));
    if (!presenter)
        return NULL;

    presenter->view = view;
    presenter->is_complete = false;

    presenter->data.hour = DEFAULT_HOUR;
    presenter->data.minute = DEFAULT_MINUTE;
    presenter->data.active_field = 0;

    presenter->time_hour_index = DEFAULT_HOUR;
    presenter->time_minute_index = DEFAULT_MINUTE;

    return presenter;
}

void SetTimePresenter_Deinit(SetTimePresenter_t *presenter)
{
    if (presenter)
        free(presenter);
}

void SetTimePresenter_HandleEvent(SetTimePresenter_t *presenter, const Input2VPEvent_t *event)
{
    if (!presenter || !event)
        return;

    bool state_changed = false;

    if (event->type == EVT_CTRL_WHEEL_DELTA)
    {
        int16_t delta = event->delta;
        
        if (presenter->data.active_field == 0)
        {
            /* Hour adjustment */
            int16_t new_hour = (int16_t)presenter->time_hour_index + delta;
            if (new_hour < 0)
                new_hour = HOURS_COUNT - 1;
            else if (new_hour >= HOURS_COUNT)
                new_hour = 0;
            presenter->time_hour_index = (uint8_t)new_hour;
            presenter->data.hour = presenter->time_hour_index;
        }
        else if (presenter->data.active_field == 1)
        {
            /* Minute adjustment */
            int16_t new_minute = (int16_t)presenter->time_minute_index + delta;
            if (new_minute < 0)
                new_minute = MINUTES_COUNT - 1;
            else if (new_minute >= MINUTES_COUNT)
                new_minute = 0;
            presenter->time_minute_index = (uint8_t)new_minute;
            presenter->data.minute = presenter->time_minute_index;
        }
        state_changed = true;
    }
    else if (event->type == EVT_MIDDLE_BTN && event->button_action == BUTTON_ACTION_PRESSED)
    {
        if (presenter->data.active_field < 1)
        {
            presenter->data.active_field++;
        }
        else
        {
            presenter->is_complete = true;
        }
        state_changed = true;
    }
    else if (event->type == EVT_LEFT_BTN && event->button_action == BUTTON_ACTION_PRESSED)
    {
        if (presenter->data.active_field > 0)
        {
            presenter->data.active_field--;
            state_changed = true;
        }
    }

    if (state_changed && presenter->view)
    {
        SetTimeView_Render(presenter->view, &presenter->data);
    }
}

void SetTimePresenter_Reset(SetTimePresenter_t *presenter)
{
    if (presenter)
    {
        presenter->is_complete = false;
    }
}

bool SetTimePresenter_IsComplete(SetTimePresenter_t *presenter)
{
    if (!presenter)
        return false;
    return presenter->is_complete;
}

const SetTime_ViewModelData_t* SetTimePresenter_GetData(SetTimePresenter_t *presenter)
{
    if (!presenter)
        return NULL;
    return &presenter->data;
}
