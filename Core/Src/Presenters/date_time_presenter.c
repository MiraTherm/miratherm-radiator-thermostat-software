#include "date_time_presenter.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "input_task.h"

/**
 * @brief Internal presenter structure
 */
typedef struct DateTimePresenter
{
    DateTime_ViewModelData_t data;
    uint8_t current_page;        /* 0: date, 1: time, 2: summer time */
    bool is_complete;
    
    /* Current roller selection indices for each page */
    uint8_t date_day_index;      /* Index in day values (0-30 for 1-31) */
    uint8_t date_month_index;    /* Index in month values (0-11 for 1-12) */
    uint8_t date_year_index;     /* Index in year values */
    
    uint8_t time_hour_index;     /* Index in hour values (0-23) */
    uint8_t time_minute_index;   /* Index in minute values (0-59) */
    
    /* Wheel focus tracking */
    uint8_t date_active_field;   /* 0: day, 1: month, 2: year */
    uint8_t time_active_field;   /* 0: hour, 1: minute */
} DateTimePresenter_t;

/* Helper functions for roller value management */
static const uint8_t DAYS_COUNT = 31;
static const uint8_t MONTHS_COUNT = 12;
static const uint8_t YEARS_COUNT = 30; /* e.g., 2020-2049 */
static const uint8_t HOURS_COUNT = 24;
static const uint8_t MINUTES_COUNT = 60;

static const uint16_t BASE_YEAR = 2020;
static const uint8_t DEFAULT_DAY = 1;
static const uint8_t DEFAULT_MONTH = 1;
static const uint16_t DEFAULT_YEAR = 2025;
static const uint8_t DEFAULT_HOUR = 12;
static const uint8_t DEFAULT_MINUTE = 0;

/**
 * @brief Initialize the date/time presenter
 */
DateTimePresenter_t* DateTimePresenter_Init(void)
{
    DateTimePresenter_t *presenter = (DateTimePresenter_t *)malloc(sizeof(DateTimePresenter_t));
    if (!presenter)
        return NULL;

    /* Initialize with defaults */
    presenter->data.day = DEFAULT_DAY;
    presenter->data.month = DEFAULT_MONTH;
    presenter->data.year = DEFAULT_YEAR;
    presenter->data.hour = DEFAULT_HOUR;
    presenter->data.minute = DEFAULT_MINUTE;
    presenter->data.is_summer_time = 0;
    
    presenter->current_page = 0;
    presenter->is_complete = 0;
    
    presenter->date_day_index = DEFAULT_DAY - 1;
    presenter->date_month_index = DEFAULT_MONTH - 1;
    presenter->date_year_index = DEFAULT_YEAR - BASE_YEAR;
    
    presenter->time_hour_index = DEFAULT_HOUR;
    presenter->time_minute_index = DEFAULT_MINUTE;
    presenter->date_active_field = 0;
    presenter->time_active_field = 0;

    return presenter;
}

/**
 * @brief Deinitialize the date/time presenter
 */
void DateTimePresenter_Deinit(DateTimePresenter_t *presenter)
{
    if (presenter)
        free(presenter);
}

/**
 * @brief Handle input event for the current wizard page
 */
void DateTimePresenter_HandleEvent(DateTimePresenter_t *presenter, const Input2VPEvent_t *event)
{
    if (!presenter || !event)
        return;

    if (presenter->current_page == 0)
    {
        /* Date selection page - 3 rollers: day, month, year */
        if (event->type == EVT_CTRL_WHEEL_DELTA)
        {
            /* Adjust the currently focused roller */
            int16_t delta = event->delta;
            
            if (presenter->date_active_field == 0)
            {
                /* Day adjustment */
                int16_t new_day = (int16_t)presenter->date_day_index + delta;
                if (new_day < 0)
                    new_day = DAYS_COUNT - 1;
                else if (new_day >= DAYS_COUNT)
                    new_day = 0;
                presenter->date_day_index = (uint8_t)new_day;
                presenter->data.day = presenter->date_day_index + 1;
            }
            else if (presenter->date_active_field == 1)
            {
                /* Month adjustment */
                int16_t new_month = (int16_t)presenter->date_month_index + delta;
                if (new_month < 0)
                    new_month = MONTHS_COUNT - 1;
                else if (new_month >= MONTHS_COUNT)
                    new_month = 0;
                presenter->date_month_index = (uint8_t)new_month;
                presenter->data.month = presenter->date_month_index + 1;
            }
            else if (presenter->date_active_field == 2)
            {
                /* Year adjustment */
                int16_t new_year = (int16_t)presenter->date_year_index + delta;
                if (new_year < 0)
                    new_year = YEARS_COUNT - 1;
                else if (new_year >= YEARS_COUNT)
                    new_year = 0;
                presenter->date_year_index = (uint8_t)new_year;
                presenter->data.year = BASE_YEAR + presenter->date_year_index;
            }
        }
        else if (event->type == EVT_CENTRAL_BTN && event->button_action == BUTTON_ACTION_PRESSED)
        {
            /* Central button moves to next field: Day -> Month -> Year -> next page */
            if (presenter->date_active_field < 2)
            {
                /* Move to next field (Day -> Month or Month -> Year) */
                presenter->date_active_field++;
            }
            else
            {
                /* Year confirmed, move to time page */
                presenter->current_page = 1;
            }
        }
        else if (event->type == EVT_MODE_BTN && event->button_action == BUTTON_ACTION_PRESSED)
        {
            /* Left button goes back to previous field */
            if (presenter->date_active_field > 0)
            {
                presenter->date_active_field--;
            }
        }
    }
    else if (presenter->current_page == 1)
    {
        /* Time selection page - 2 rollers: hour, minute */
        if (event->type == EVT_CTRL_WHEEL_DELTA)
        {
            int16_t delta = event->delta;
            
            if (presenter->time_active_field == 0)
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
            else if (presenter->time_active_field == 1)
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
        }
        else if (event->type == EVT_CENTRAL_BTN && event->button_action == BUTTON_ACTION_PRESSED)
        {
            /* Central button moves to next field: Hour -> Minute -> next page */
            if (presenter->time_active_field < 1)
            {
                /* Move to minute field */
                presenter->time_active_field++;
            }
            else
            {
                /* Minute confirmed, move to summer time page */
                presenter->current_page = 2;
            }
        }
        else if (event->type == EVT_MODE_BTN && event->button_action == BUTTON_ACTION_PRESSED)
        {
            /* Left button goes back to previous page or field */
            if (presenter->time_active_field > 0)
            {
                /* Go back to hour field */
                presenter->time_active_field--;
            }
            else
            {
                /* Go back to date page */
                presenter->current_page = 0;
            }
        }
    }
    else if (presenter->current_page == 2)
    {
        /* Summer time toggle page - encoder and central button only */
        if (event->type == EVT_CTRL_WHEEL_DELTA)
        {
            /* Encoder left (negative delta) -> OFF, right (positive delta) -> ON */
            if (event->delta < 0)
            {
                presenter->data.is_summer_time = 0;
            }
            else if (event->delta > 0)
            {
                presenter->data.is_summer_time = 1;
            }
        }
        else if ((event->type == EVT_CENTRAL_BTN || event->type == EVT_CENTRAL_DOUBLE_CLICK) && event->button_action == BUTTON_ACTION_PRESSED)
        {
            /* Central button confirms and completes the wizard */
            presenter->is_complete = 1;
        }
        else if (event->type == EVT_MODE_BTN && event->button_action == BUTTON_ACTION_PRESSED)
        {
            /* Left button goes back to previous page */
            presenter->current_page = 1;
        }
    }
}

/**
 * @brief Get current wizard page (0, 1, or 2)
 */
uint8_t DateTimePresenter_GetCurrentPage(DateTimePresenter_t *presenter)
{
    if (!presenter)
        return 0;
    return presenter->current_page;
}

/**
 * @brief Get active field on date selection page (0: day, 1: month, 2: year)
 */
uint8_t DateTimePresenter_GetDateActiveField(DateTimePresenter_t *presenter)
{
    if (!presenter)
        return 0;
    return presenter->date_active_field;
}

/**
 * @brief Get active field on time selection page (0: hour, 1: minute)
 */
uint8_t DateTimePresenter_GetTimeActiveField(DateTimePresenter_t *presenter)
{
    if (!presenter)
        return 0;
    return presenter->time_active_field;
}

/**
 * @brief Check if configuration is complete (all 3 pages done)
 */
bool DateTimePresenter_IsComplete(DateTimePresenter_t *presenter)
{
    if (!presenter)
        return false;
    return presenter->is_complete;
}

/**
 * @brief Get the current data
 */
const DateTime_ViewModelData_t* DateTimePresenter_GetData(DateTimePresenter_t *presenter)
{
    if (!presenter)
        return NULL;
    return &presenter->data;
}

/**
 * @brief Update view - called when state changes
 */
void DateTimePresenter_OnViewUpdateNeeded(DateTimePresenter_t *presenter)
{
    /* This would trigger a view render in a full implementation */
    (void)presenter;
}
