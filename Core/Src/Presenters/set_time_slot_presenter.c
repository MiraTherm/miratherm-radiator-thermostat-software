#include "set_time_slot_presenter.h"
#include "set_time_slot_view.h"
#include <stdlib.h>

typedef struct SetTimeSlotPresenter
{
    SetTimeSlotView_t *view;
    SetTimeSlot_ViewModelData_t data;
    bool is_complete;
} SetTimeSlotPresenter_t;

static const uint8_t HOURS_COUNT = 24;
static const uint8_t MINUTES_COUNT = 12;  /* 5-minute resolution: 0, 5, 10, ..., 55 minutes */
static const uint8_t MINUTE_STEP = 5;

SetTimeSlotPresenter_t* SetTimeSlotPresenter_Init(SetTimeSlotView_t *view)
{
    SetTimeSlotPresenter_t *presenter = (SetTimeSlotPresenter_t *)malloc(sizeof(SetTimeSlotPresenter_t));
    if (!presenter) return NULL;

    presenter->view = view;
    presenter->is_complete = false;
    
    /* Default data */
    presenter->data.start_hour = 0;
    presenter->data.start_minute = 0;
    presenter->data.end_hour = 0;
    presenter->data.end_minute = 0;
    presenter->data.active_field = 0;
    presenter->data.start_time_locked = false;
    presenter->data.end_time_locked = false;

    return presenter;
}

void SetTimeSlotPresenter_Deinit(SetTimeSlotPresenter_t *presenter)
{
    if (presenter) free(presenter);
}

void SetTimeSlotPresenter_HandleEvent(SetTimeSlotPresenter_t *presenter, const Input2VPEvent_t *event)
{
    if (!presenter || !event) return;

    bool state_changed = false;

    if (event->type == EVT_CTRL_WHEEL_DELTA)
    {
        int16_t delta = event->delta;
        
        if (presenter->data.active_field == 0 && !presenter->data.start_time_locked)
        {
            int16_t val = (int16_t)presenter->data.start_hour + delta;
            if (val < 0) val = HOURS_COUNT - 1;
            else if (val >= HOURS_COUNT) val = 0;
            presenter->data.start_hour = (uint8_t)val;
            state_changed = true;
        }
        else if (presenter->data.active_field == 1 && !presenter->data.start_time_locked)
        {
            int16_t val = (int16_t)(presenter->data.start_minute / MINUTE_STEP) + delta;
            if (val < 0) val = MINUTES_COUNT - 1;
            else if (val >= MINUTES_COUNT) val = 0;
            presenter->data.start_minute = (uint8_t)(val * MINUTE_STEP);
            state_changed = true;
        }
        else if (presenter->data.active_field == 2 && !presenter->data.end_time_locked)
        {
            int16_t val = (int16_t)presenter->data.end_hour + delta;
            if (val < 0) val = HOURS_COUNT - 1;
            else if (val >= HOURS_COUNT) val = 0;
            
            /* Prevent end time from being less than or equal to start time */
            /* If end hour would be less than start hour, don't allow it */
            if (val < presenter->data.start_hour)
            {
                val = presenter->data.start_hour;
            }
            /* If end hour equals start hour, ensure end minute > start minute */
            else if (val == presenter->data.start_hour && presenter->data.end_minute <= presenter->data.start_minute)
            {
                /* Set end_minute to be > start_minute (next 5-minute interval) */
                uint8_t next_minute = presenter->data.start_minute + MINUTE_STEP;
                if (next_minute <= 55)
                {
                    presenter->data.end_minute = next_minute;
                }
                else
                {
                    /* Next interval would overflow hour, don't change end hour */
                    return;
                }
            }
            
            presenter->data.end_hour = (uint8_t)val;
            state_changed = true;
        }
        else if (presenter->data.active_field == 3 && !presenter->data.end_time_locked)
        {
            int16_t val = (int16_t)(presenter->data.end_minute / MINUTE_STEP) + delta;
            if (val < 0) val = MINUTES_COUNT - 1;
            else if (val >= MINUTES_COUNT) val = 0;
            
            uint8_t new_end_minute = (uint8_t)(val * MINUTE_STEP);
            
            /* Prevent end time from being less than or equal to start time */
            /* If we're in the same hour as start time, don't allow end_minute <= start_minute */
            if (presenter->data.end_hour == presenter->data.start_hour)
            {
                if (new_end_minute <= presenter->data.start_minute)
                {
                    /* Don't allow this value, stay at current or go to next valid value */
                    /* Find the next valid minute that is > start_minute */
                    uint8_t next_minute = presenter->data.start_minute + MINUTE_STEP;
                    if (next_minute <= 55)
                    {
                        val = next_minute / MINUTE_STEP;
                        presenter->data.end_minute = next_minute;
                    }
                    else
                    {
                        /* No valid minute in this hour, would need to go to next hour */
                        return;
                    }
                }
                else
                {
                    presenter->data.end_minute = new_end_minute;
                }
            }
            else
            {
                presenter->data.end_minute = new_end_minute;
            }
            
            state_changed = true;
        }
    }
    else if (event->type == EVT_MIDDLE_BTN && event->button_action == BUTTON_ACTION_PRESSED)
    {
        /* Move to next field or complete */
        /* Skip locked fields if we are moving forward? Or just pass through them? */
        /* Requirement says: "On the first time slot begin time (always 00:00) should be blocked." */
        /* This implies we shouldn't even edit it. */
        
        /* Simple logic: increment active field until we find an unlocked one or finish */
        
        uint8_t next_field = presenter->data.active_field + 1;
        
        /* Check if next fields are locked, if so skip them? 
           Actually, if start is locked, we probably start at field 2 (EndHH).
           But here we are handling the transition.
        */
        
        if (presenter->data.active_field == 2 && presenter->data.end_time_locked)
        {
            /* If we are at field 2 and end time is locked, pressing OK should complete */
            presenter->is_complete = true; /* Skip end time editing */
        }
        else if (next_field > 3)
        {
            presenter->is_complete = true;
        }
        else
        {
            presenter->data.active_field = next_field;
        }
        state_changed = true;
    }
    else if (event->type == EVT_LEFT_BTN && event->button_action == BUTTON_ACTION_PRESSED)
    {
        /* Back navigation within fields */
        if (presenter->data.active_field > 0)
        {
            uint8_t prev_field = presenter->data.active_field - 1;
            
            /* If we are at field 2 and go back, and start time is locked (fields 0,1 locked), 
               we should probably not go there? 
               But the "back_hint" requirement usually means going back to previous screen if at first field.
               Here we are talking about fields.
            */
            
            if (prev_field == 1 && presenter->data.start_time_locked)
            {
                /* Start time locked means 0 and 1 are locked. So we can't go back to 1. */
                /* We are at 2 (EndHH). If we press back, we should probably exit the presenter (handled by router) 
                   OR stay at 2. 
                   But usually MODE button at first editable field is handled by router to go back to previous step.
                   So here we only handle if we can go back to a field.
                */
            }
            else
            {
                presenter->data.active_field = prev_field;
                state_changed = true;
            }
        }
    }

    if (state_changed)
    {
        SetTimeSlotView_Render(presenter->view, &presenter->data);
    }
}

bool SetTimeSlotPresenter_IsComplete(SetTimeSlotPresenter_t *presenter)
{
    return presenter ? presenter->is_complete : false;
}

SetTimeSlot_ViewModelData_t SetTimeSlotPresenter_GetData(SetTimeSlotPresenter_t *presenter)
{
    if (presenter) return presenter->data;
    SetTimeSlot_ViewModelData_t empty = {0};
    return empty;
}

void SetTimeSlotPresenter_SetData(SetTimeSlotPresenter_t *presenter, const SetTimeSlot_ViewModelData_t *data)
{
    if (presenter && data)
    {
        presenter->data = *data;
        
        /* Ensure active field is valid (point to first unlocked field) */
        if (presenter->data.start_time_locked)
        {
            presenter->data.active_field = 2; /* Start at End Hour */
        }
        else
        {
            presenter->data.active_field = 0;
        }
        
        SetTimeSlotView_Render(presenter->view, &presenter->data);
    }
}

void SetTimeSlotPresenter_Reset(SetTimeSlotPresenter_t *presenter)
{
    if (presenter)
    {
        presenter->is_complete = false;
        /* Reset active field based on locks */
        if (presenter->data.start_time_locked)
            presenter->data.active_field = 2;
        else
            presenter->data.active_field = 0;
            
        SetTimeSlotView_Render(presenter->view, &presenter->data);
    }
}
