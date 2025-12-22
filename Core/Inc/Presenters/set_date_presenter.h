#ifndef CORE_INC_PRESENTERS_SET_DATE_PRESENTER_H
#define CORE_INC_PRESENTERS_SET_DATE_PRESENTER_H

#include "set_date_viewmodel.h"
#include "input_task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetDatePresenter SetDatePresenter_t;
typedef struct SetDateView SetDateView_t;

SetDatePresenter_t* SetDatePresenter_Init(SetDateView_t *view);
void SetDatePresenter_Deinit(SetDatePresenter_t *presenter);
void SetDatePresenter_HandleEvent(SetDatePresenter_t *presenter, const Input2VPEvent_t *event);
void SetDatePresenter_Reset(SetDatePresenter_t *presenter);
bool SetDatePresenter_IsComplete(SetDatePresenter_t *presenter);
const SetDate_ViewModelData_t* SetDatePresenter_GetData(SetDatePresenter_t *presenter);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_SET_DATE_PRESENTER_H */
