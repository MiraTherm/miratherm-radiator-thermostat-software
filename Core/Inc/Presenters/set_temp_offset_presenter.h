#ifndef CORE_INC_PRESENTERS_SET_TEMP_OFFSET_PRESENTER_H
#define CORE_INC_PRESENTERS_SET_TEMP_OFFSET_PRESENTER_H

#include "set_value_view.h"
#include "input_task.h"
#include "storage_task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetTempOffsetPresenter SetTempOffsetPresenter_t;

SetTempOffsetPresenter_t* SetTempOffsetPresenter_Init(SetValueView_t *view, ConfigModel_t *config_access);
void SetTempOffsetPresenter_Deinit(SetTempOffsetPresenter_t *presenter);
void SetTempOffsetPresenter_HandleEvent(SetTempOffsetPresenter_t *presenter, const Input2VPEvent_t *event);
bool SetTempOffsetPresenter_IsComplete(SetTempOffsetPresenter_t *presenter);
bool SetTempOffsetPresenter_IsCancelled(SetTempOffsetPresenter_t *presenter);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_SET_TEMP_OFFSET_PRESENTER_H */
