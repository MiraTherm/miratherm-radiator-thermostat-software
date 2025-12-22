#ifndef CORE_INC_PRESENTERS_SET_VALUE_PRESENTER_H
#define CORE_INC_PRESENTERS_SET_VALUE_PRESENTER_H

#include "set_value_viewmodel.h"
#include "input_task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetValuePresenter SetValuePresenter_t;
typedef struct SetValueView SetValueView_t;

SetValuePresenter_t* SetValuePresenter_Init(SetValueView_t *view, uint16_t initial_index, uint16_t max_index);
void SetValuePresenter_Deinit(SetValuePresenter_t *presenter);
void SetValuePresenter_HandleEvent(SetValuePresenter_t *presenter, const Input2VPEvent_t *event);
bool SetValuePresenter_IsComplete(SetValuePresenter_t *presenter);
uint16_t SetValuePresenter_GetSelectedIndex(SetValuePresenter_t *presenter);
void SetValuePresenter_Reset(SetValuePresenter_t *presenter);
void SetValuePresenter_SetMaxIndex(SetValuePresenter_t *presenter, uint16_t max_index);
void SetValuePresenter_SetSelectedIndex(SetValuePresenter_t *presenter, uint16_t index);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_SET_VALUE_PRESENTER_H */
