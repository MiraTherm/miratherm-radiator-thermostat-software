#include "date_time_presenter.h"
#include "date_time_view.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "input_task.h"
#include "stm32wbxx_hal.h"

/**
 * @brief External RTC handle from main.c
 */
extern RTC_HandleTypeDef hrtc;

/**
 * @brief Internal presenter structure
 */
typedef struct DateTimePresenter
{
    DateTimeView_t *view;        /* Reference to the view */
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
    uint8_t date_active_field;   /* 0: year, 1: month, 2: day */
    uint8_t time_active_field;   /* 0: hour, 1: minute */
} DateTimePresenter_t;

/* Helper functions for roller value management */
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
 * @brief Check if a year is a leap year
 */
static bool is_leap_year(uint16_t year)
{
    /* A year is a leap year if:
     * - It is divisible by 4 AND
     * - (It is not divisible by 100 OR it is divisible by 400)
     */
    if ((year % 4) != 0)
        return false;
    if ((year % 100) != 0)
        return true;
    if ((year % 400) == 0)
        return true;
    return false;
}

/**
 * @brief Get the maximum number of days in a given month/year
 */
static uint8_t get_max_days_in_month(uint8_t month, uint16_t year)
{
    /* Month should be 1-12 */
    if (month < 1 || month > 12)
        return 31;

    switch (month)
    {
        case 1:  /* January */
        case 3:  /* March */
        case 5:  /* May */
        case 7:  /* July */
        case 8:  /* August */
        case 10: /* October */
        case 12: /* December */
            return 31;

        case 4:  /* April */
        case 6:  /* June */
        case 9:  /* September */
        case 11: /* November */
            return 30;

        case 2:  /* February */
            return is_leap_year(year) ? 29 : 28;

        default:
            return 31;
    }
}

/**
 * @brief Validate and adjust day if it exceeds the maximum for the given month/year
 */
static void validate_and_adjust_day(DateTimePresenter_t *presenter)
{
    if (!presenter)
        return;

    uint8_t max_days = get_max_days_in_month(presenter->data.month, presenter->data.year);

    if (presenter->data.day > max_days)
    {
        /* Adjust day to maximum valid day for this month */
        presenter->data.day = max_days;
        /* Also update the index (day is 1-31, index is 0-30) */
        presenter->date_day_index = max_days - 1;
    }
}

/**
 * @brief Set the RTC with the configured date and time
 */
void set_rtc(DateTimePresenter_t *presenter)
{
    if (!presenter)
        return;

    /* Check if RTC is initialized */
    if (hrtc.Instance == NULL)
    {
        return;
    }

    /* Initialize RTC structures */
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    /* Set date from presenter data */
    sDate.Year = (uint8_t)(presenter->data.year - 2000);  /* RTC year is 00-99 (2000-2099) */
    sDate.Month = presenter->data.month;
    sDate.Date = presenter->data.day;
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;  /* Default to Monday, can be calculated if needed */

    /* Set time from presenter data */
    sTime.Hours = presenter->data.hour;
    sTime.Minutes = presenter->data.minute;
    sTime.Seconds = 0;  /* Default to 0 seconds */
    sTime.TimeFormat = RTC_HOURFORMAT_24;
    sTime.DayLightSaving = (presenter->data.is_summer_time) ? RTC_DAYLIGHTSAVING_ADD1H : RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    /* Set the time */
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        /* Error occurred, could log this */
        return;
    }

    /* Set the date */
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        /* Error occurred, could log this */
        return;
    }
}

/**
 * @brief Initialize the date/time presenter
 */
DateTimePresenter_t* DateTimePresenter_Init(DateTimeView_t *view)
{
    DateTimePresenter_t *presenter = (DateTimePresenter_t *)malloc(sizeof(DateTimePresenter_t));
    if (!presenter)
        return NULL;

    presenter->view = view;

    /* Initialize with defaults */
    presenter->data.day = DEFAULT_DAY;
    presenter->data.month = DEFAULT_MONTH;
    presenter->data.year = DEFAULT_YEAR;
    presenter->data.hour = DEFAULT_HOUR;
    presenter->data.minute = DEFAULT_MINUTE;
    presenter->data.is_summer_time = 0;
    presenter->data.current_page = 0;
    presenter->data.date_active_field = 0;
    presenter->data.time_active_field = 0;
    
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

    bool state_changed = false;

    if (presenter->current_page == 0)
    {
        /* Date selection page - 3 rollers: year, month, day */
        if (event->type == EVT_CTRL_WHEEL_DELTA)
        {
            /* Adjust the currently focused roller */
            int16_t delta = event->delta;
            
            if (presenter->date_active_field == 0)
            {
                /* Year adjustment */
                int16_t new_year = (int16_t)presenter->date_year_index + delta;
                if (new_year < 0)
                    new_year = YEARS_COUNT - 1;
                else if (new_year >= YEARS_COUNT)
                    new_year = 0;
                presenter->date_year_index = (uint8_t)new_year;
                presenter->data.year = BASE_YEAR + presenter->date_year_index;
                
                /* Validate and adjust day if leap year status changed (affects February) */
                validate_and_adjust_day(presenter);
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
                
                /* Validate and adjust day if it exceeds max days in the new month */
                validate_and_adjust_day(presenter);
            }
            else if (presenter->date_active_field == 2)
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
            /* Central button moves to next field: Year -> Month -> Day -> next page */
            if (presenter->date_active_field < 2)
            {
                /* Move to next field (Year -> Month or Month -> Day) */
                presenter->date_active_field++;
            }
            else
            {
                /* Day confirmed, move to time page */
                presenter->current_page = 1;
                presenter->date_active_field = 0;  /* Reset to first field on new page */
            }
            state_changed = true;
        }
        else if (event->type == EVT_MODE_BTN && event->button_action == BUTTON_ACTION_PRESSED)
        {
            /* Left button goes back to previous field */
            if (presenter->date_active_field > 0)
            {
                presenter->date_active_field--;
                state_changed = true;
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
            state_changed = true;
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
                presenter->time_active_field = 0;  /* Reset for new page */
            }
            state_changed = true;
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
                presenter->date_active_field = 0;  /* Reset to first field */
            }
            state_changed = true;
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
            state_changed = true;
        }
        else if ((event->type == EVT_CENTRAL_BTN || event->type == EVT_CENTRAL_DOUBLE_CLICK) && event->button_action == BUTTON_ACTION_PRESSED)
        {
            /* Central button confirms and completes the wizard */
            set_rtc(presenter);
            presenter->is_complete = 1;
        }
        else if (event->type == EVT_MODE_BTN && event->button_action == BUTTON_ACTION_PRESSED)
        {
            /* Left button goes back to previous page */
            presenter->current_page = 1;
            presenter->time_active_field = 0;  /* Reset to first field */
            state_changed = true;
        }
    }

    /* If state changed, sync to ViewModel and render */
    if (state_changed)
    {
        presenter->data.current_page = presenter->current_page;
        presenter->data.date_active_field = presenter->date_active_field;
        presenter->data.time_active_field = presenter->time_active_field;
        
        if (presenter->view)
        {
            DateTimeView_Render(presenter->view, &presenter->data);
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
/**
 * @brief Periodic run/tick for presenter updates
 */
void DateTimePresenter_Run(DateTimePresenter_t *presenter)
{
    if (!presenter || !presenter->view)
        return;

    /* Sync internal state to ViewModel */
    presenter->data.current_page = presenter->current_page;
    presenter->data.date_active_field = presenter->date_active_field;
    presenter->data.time_active_field = presenter->time_active_field;

    /* Render the current state to the view */
    DateTimeView_Render(presenter->view, &presenter->data);
}