#include "set_date_presenter.h"
#include "set_date_view.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct SetDatePresenter
{
    SetDateView_t *view;
    SetDate_ViewModelData_t data;
    bool is_complete;
    
    uint8_t date_day_index;
    uint8_t date_month_index;
    uint8_t date_year_index;
} SetDatePresenter_t;

static const uint8_t MONTHS_COUNT = 12;
static const uint8_t YEARS_COUNT = 30; /* e.g., 2020-2049 */
static const uint16_t BASE_YEAR = 2020;
static const uint8_t DEFAULT_DAY = 1;
static const uint8_t DEFAULT_MONTH = 1;
static const uint16_t DEFAULT_YEAR = 2025;

static bool is_leap_year(uint16_t year)
{
    if ((year % 4) != 0)
        return false;
    if ((year % 100) != 0)
        return true;
    if ((year % 400) == 0)
        return true;
    return false;
}

static uint8_t get_max_days_in_month(uint8_t month, uint16_t year)
{
    if (month < 1 || month > 12)
        return 31;

    switch (month)
    {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return is_leap_year(year) ? 29 : 28;
        default:
            return 31;
    }
}

static void validate_and_adjust_day(SetDatePresenter_t *presenter)
{
    if (!presenter)
        return;

    uint8_t max_days = get_max_days_in_month(presenter->data.month, presenter->data.year);

    if (presenter->data.day > max_days)
    {
        presenter->data.day = max_days;
        presenter->date_day_index = max_days - 1;
    }
}

SetDatePresenter_t* SetDatePresenter_Init(SetDateView_t *view)
{
    SetDatePresenter_t *presenter = (SetDatePresenter_t *)malloc(sizeof(SetDatePresenter_t));
    if (!presenter)
        return NULL;

    presenter->view = view;
    presenter->is_complete = false;

    presenter->data.day = DEFAULT_DAY;
    presenter->data.month = DEFAULT_MONTH;
    presenter->data.year = DEFAULT_YEAR;
    presenter->data.active_field = 0;

    presenter->date_day_index = DEFAULT_DAY - 1;
    presenter->date_month_index = DEFAULT_MONTH - 1;
    presenter->date_year_index = DEFAULT_YEAR - BASE_YEAR;

    return presenter;
}

void SetDatePresenter_Deinit(SetDatePresenter_t *presenter)
{
    if (presenter)
        free(presenter);
}

void SetDatePresenter_HandleEvent(SetDatePresenter_t *presenter, const Input2VPEvent_t *event)
{
    if (!presenter || !event)
        return;

    bool state_changed = false;

    if (event->type == EVT_CTRL_WHEEL_DELTA)
    {
        int16_t delta = event->delta;
        
        if (presenter->data.active_field == 0)
        {
            /* Year adjustment */
            int16_t new_year = (int16_t)presenter->date_year_index + delta;
            if (new_year < 0)
                new_year = YEARS_COUNT - 1;
            else if (new_year >= YEARS_COUNT)
                new_year = 0;
            presenter->date_year_index = (uint8_t)new_year;
            presenter->data.year = BASE_YEAR + presenter->date_year_index;
            
            validate_and_adjust_day(presenter);
        }
        else if (presenter->data.active_field == 1)
        {
            /* Month adjustment */
            int16_t new_month = (int16_t)presenter->date_month_index + delta;
            if (new_month < 0)
                new_month = MONTHS_COUNT - 1;
            else if (new_month >= MONTHS_COUNT)
                new_month = 0;
            presenter->date_month_index = (uint8_t)new_month;
            presenter->data.month = presenter->date_month_index + 1;
            
            validate_and_adjust_day(presenter);
        }
        else if (presenter->data.active_field == 2)
        {
            /* Day adjustment */
            uint8_t max_days = get_max_days_in_month(presenter->data.month, presenter->data.year);
            int16_t new_day = (int16_t)presenter->date_day_index + delta;
            if (new_day < 0)
                new_day = max_days - 1;
            else if (new_day >= max_days)
                new_day = 0;
            presenter->date_day_index = (uint8_t)new_day;
            presenter->data.day = presenter->date_day_index + 1;
        }
        state_changed = true;
    }
    else if (event->type == EVT_CENTRAL_BTN && event->button_action == BUTTON_ACTION_PRESSED)
    {
        if (presenter->data.active_field < 2)
        {
            presenter->data.active_field++;
        }
        else
        {
            presenter->is_complete = true;
        }
        state_changed = true;
    }
    else if (event->type == EVT_MODE_BTN && event->button_action == BUTTON_ACTION_PRESSED)
    {
        if (presenter->data.active_field > 0)
        {
            presenter->data.active_field--;
            state_changed = true;
        }
    }

    if (state_changed && presenter->view)
    {
        SetDateView_Render(presenter->view, &presenter->data);
    }
}

void SetDatePresenter_Reset(SetDatePresenter_t *presenter)
{
    if (presenter)
    {
        presenter->is_complete = false;
    }
}

bool SetDatePresenter_IsComplete(SetDatePresenter_t *presenter)
{
    if (!presenter)
        return false;
    return presenter->is_complete;
}

const SetDate_ViewModelData_t* SetDatePresenter_GetData(SetDatePresenter_t *presenter)
{
    if (!presenter)
        return NULL;
    return &presenter->data;
}
