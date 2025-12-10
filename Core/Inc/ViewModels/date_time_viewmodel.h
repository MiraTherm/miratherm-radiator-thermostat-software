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
    uint8_t day;      /* 1-31 */
    uint8_t month;    /* 1-12 */
    uint16_t year;    /* e.g., 2025 */
    uint8_t hour;     /* 0-23 */
    uint8_t minute;   /* 0-59 */
    bool is_summer_time; /* DST flag */
} DateTime_ViewModelData_t;

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWMODELS_DATE_TIME_VIEWMODEL_H */
