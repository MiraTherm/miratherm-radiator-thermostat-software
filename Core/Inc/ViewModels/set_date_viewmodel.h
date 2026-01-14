#ifndef CORE_INC_VIEWMODELS_SET_DATE_VIEWMODEL_H
#define CORE_INC_VIEWMODELS_SET_DATE_VIEWMODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t day;      /* 1-31 */
    uint8_t month;    /* 1-12 */
    uint16_t year;    /* e.g., 2026 */
    uint8_t active_field;   /* 0: year, 1: month, 2: day */
} SetDate_ViewModelData_t;

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWMODELS_SET_DATE_VIEWMODEL_H */
