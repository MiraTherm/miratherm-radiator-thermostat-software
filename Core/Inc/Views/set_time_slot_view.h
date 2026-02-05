#ifndef CORE_INC_VIEWS_SET_TIME_SLOT_VIEW_H
#define CORE_INC_VIEWS_SET_TIME_SLOT_VIEW_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef SetTimeSlotViewData_t
 * @brief View data for time slot setting screen
 */
typedef struct
{
    uint8_t start_hour;
    uint8_t start_minute;
    uint8_t end_hour;
    uint8_t end_minute;
    uint8_t active_field; /* 0: StartHH, 1: StartMM, 2: EndHH, 3: EndMM */
    bool start_time_locked;
    bool end_time_locked;
} SetTimeSlotViewData_t;

typedef struct SetTimeSlotView SetTimeSlotView_t;

SetTimeSlotView_t* SetTimeSlotView_Init(const char *title);
void SetTimeSlotView_Deinit(SetTimeSlotView_t *view);
void SetTimeSlotView_Render(SetTimeSlotView_t *view, const SetTimeSlotViewData_t *data);
void SetTimeSlotView_Show(SetTimeSlotView_t *view);
void SetTimeSlotView_Hide(SetTimeSlotView_t *view);
void SetTimeSlotView_SetTitle(SetTimeSlotView_t *view, const char *title);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_SET_TIME_SLOT_VIEW_H */
