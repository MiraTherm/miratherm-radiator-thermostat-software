#ifndef CORE_INC_PRESENTERS_SET_TIME_SLOT_PRESENTER_H
#define CORE_INC_PRESENTERS_SET_TIME_SLOT_PRESENTER_H

#include "set_time_slot_viewmodel.h"
#include "input_task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetTimeSlotPresenter SetTimeSlotPresenter_t;
typedef struct SetTimeSlotView SetTimeSlotView_t;

SetTimeSlotPresenter_t* SetTimeSlotPresenter_Init(SetTimeSlotView_t *view);
void SetTimeSlotPresenter_Deinit(SetTimeSlotPresenter_t *presenter);
void SetTimeSlotPresenter_HandleEvent(SetTimeSlotPresenter_t *presenter, const Input2VPEvent_t *event);
bool SetTimeSlotPresenter_IsComplete(SetTimeSlotPresenter_t *presenter);
SetTimeSlot_ViewModelData_t SetTimeSlotPresenter_GetData(SetTimeSlotPresenter_t *presenter);
void SetTimeSlotPresenter_SetData(SetTimeSlotPresenter_t *presenter, const SetTimeSlot_ViewModelData_t *data);
void SetTimeSlotPresenter_Reset(SetTimeSlotPresenter_t *presenter);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_SET_TIME_SLOT_PRESENTER_H */
