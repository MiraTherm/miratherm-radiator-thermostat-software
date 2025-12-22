#ifndef CORE_INC_VIEWS_CHANGE_SCHEDULE_VIEW_H
#define CORE_INC_VIEWS_CHANGE_SCHEDULE_VIEW_H

#include "set_bool_view.h"
#include "set_value_view.h"
#include "set_time_slot_view.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ChangeScheduleView ChangeScheduleView_t;

ChangeScheduleView_t* ChangeScheduleView_Init(void);
void ChangeScheduleView_Deinit(ChangeScheduleView_t *view);

SetBoolView_t* ChangeScheduleView_GetBoolView(ChangeScheduleView_t *view);
SetValueView_t* ChangeScheduleView_GetValueView(ChangeScheduleView_t *view);
SetTimeSlotView_t* ChangeScheduleView_GetTimeSlotView(ChangeScheduleView_t *view);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_CHANGE_SCHEDULE_VIEW_H */
