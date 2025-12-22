#ifndef CORE_INC_VIEWMODELS_SET_TIME_SLOT_VIEWMODEL_H
#define CORE_INC_VIEWMODELS_SET_TIME_SLOT_VIEWMODEL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t start_hour;
    uint8_t start_minute;
    uint8_t end_hour;
    uint8_t end_minute;
    uint8_t active_field; /* 0: StartHH, 1: StartMM, 2: EndHH, 3: EndMM */
    bool start_time_locked;
    bool end_time_locked;
} SetTimeSlot_ViewModelData_t;

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWMODELS_SET_TIME_SLOT_VIEWMODEL_H */
