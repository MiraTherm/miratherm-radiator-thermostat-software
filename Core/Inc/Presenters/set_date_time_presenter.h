#ifndef CORE_INC_PRESENTERS_SET_DATE_TIME_PRESENTER_H
#define CORE_INC_PRESENTERS_SET_DATE_TIME_PRESENTER_H

#include "set_date_time_view.h"
#include "input_task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetDateTimePresenter SetDateTimePresenter_t;

SetDateTimePresenter_t* SetDateTimePresenter_Init(SetDateTimeView_t *view);
void SetDateTimePresenter_Deinit(SetDateTimePresenter_t *presenter);
void SetDateTimePresenter_HandleEvent(SetDateTimePresenter_t *presenter, const Input2VPEvent_t *event);
bool SetDateTimePresenter_IsComplete(SetDateTimePresenter_t *presenter);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_SET_DATE_TIME_PRESENTER_H */
