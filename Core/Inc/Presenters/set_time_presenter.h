#ifndef CORE_INC_PRESENTERS_SET_TIME_PRESENTER_H
#define CORE_INC_PRESENTERS_SET_TIME_PRESENTER_H

#include "set_time_viewmodel.h"
#include "input_task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetTimePresenter SetTimePresenter_t;
typedef struct SetTimeView SetTimeView_t;

SetTimePresenter_t* SetTimePresenter_Init(SetTimeView_t *view);
void SetTimePresenter_Deinit(SetTimePresenter_t *presenter);
void SetTimePresenter_HandleEvent(SetTimePresenter_t *presenter, const Input2VPEvent_t *event);
void SetTimePresenter_Reset(SetTimePresenter_t *presenter);
bool SetTimePresenter_IsComplete(SetTimePresenter_t *presenter);
const SetTime_ViewModelData_t* SetTimePresenter_GetData(SetTimePresenter_t *presenter);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_SET_TIME_PRESENTER_H */
