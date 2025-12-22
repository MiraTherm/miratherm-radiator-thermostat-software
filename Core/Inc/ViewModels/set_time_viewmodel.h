#ifndef CORE_INC_VIEWMODELS_SET_TIME_VIEWMODEL_H
#define CORE_INC_VIEWMODELS_SET_TIME_VIEWMODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t hour;     /* 0-23 */
    uint8_t minute;   /* 0-59 */
    uint8_t active_field;   /* 0: hour, 1: minute */
} SetTime_ViewModelData_t;

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWMODELS_SET_TIME_VIEWMODEL_H */
