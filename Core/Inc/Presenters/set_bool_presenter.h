#ifndef CORE_INC_PRESENTERS_SET_BOOL_PRESENTER_H
#define CORE_INC_PRESENTERS_SET_BOOL_PRESENTER_H

#include "set_bool_viewmodel.h"
#include "input_task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetBoolPresenter SetBoolPresenter_t;
typedef struct SetBoolView SetBoolView_t;

SetBoolPresenter_t* SetBoolPresenter_Init(SetBoolView_t *view);
void SetBoolPresenter_Deinit(SetBoolPresenter_t *presenter);
void SetBoolPresenter_HandleEvent(SetBoolPresenter_t *presenter, const Input2VPEvent_t *event);
void SetBoolPresenter_Reset(SetBoolPresenter_t *presenter);
bool SetBoolPresenter_IsComplete(SetBoolPresenter_t *presenter);
const SetBool_ViewModelData_t* SetBoolPresenter_GetData(SetBoolPresenter_t *presenter);
void SetBoolPresenter_Run(SetBoolPresenter_t *presenter, uint32_t current_tick);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_SET_BOOL_PRESENTER_H */
