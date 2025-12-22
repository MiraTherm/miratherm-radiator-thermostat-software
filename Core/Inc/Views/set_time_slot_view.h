#ifndef CORE_INC_VIEWS_SET_TIME_SLOT_VIEW_H
#define CORE_INC_VIEWS_SET_TIME_SLOT_VIEW_H

#include "set_time_slot_viewmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetTimeSlotView SetTimeSlotView_t;

SetTimeSlotView_t* SetTimeSlotView_Init(const char *title);
void SetTimeSlotView_Deinit(SetTimeSlotView_t *view);
void SetTimeSlotView_Render(SetTimeSlotView_t *view, const SetTimeSlot_ViewModelData_t *data);
void SetTimeSlotView_Show(SetTimeSlotView_t *view);
void SetTimeSlotView_Hide(SetTimeSlotView_t *view);
void SetTimeSlotView_SetTitle(SetTimeSlotView_t *view, const char *title);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_SET_TIME_SLOT_VIEW_H */
