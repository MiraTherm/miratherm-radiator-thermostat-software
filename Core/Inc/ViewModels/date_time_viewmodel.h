#ifndef CORE_INC_VIEWMODELS_DATE_TIME_VIEWMODEL_H
#define CORE_INC_VIEWMODELS_DATE_TIME_VIEWMODEL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Date and time configuration view model
 */
typedef struct
{
    /* Data */
    uint8_t day;      /* 1-31 */
    uint8_t month;    /* 1-12 */
    uint16_t year;    /* e.g., 2025 */
    uint8_t hour;     /* 0-23 */
    uint8_t minute;   /* 0-59 */
    bool is_summer_time; /* DST flag */
    
    /* UI State */
    uint8_t current_page;        /* 0: date, 1: time, 2: summer time */
    uint8_t date_active_field;   /* 0: day, 1: month, 2: year */
    uint8_t time_active_field;   /* 0: hour, 1: minute */
} DateTime_ViewModelData_t;

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWMODELS_DATE_TIME_VIEWMODEL_H */
