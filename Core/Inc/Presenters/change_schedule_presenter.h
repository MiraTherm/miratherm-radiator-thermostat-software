#ifndef CORE_INC_PRESENTERS_CHANGE_SCHEDULE_PRESENTER_H
#define CORE_INC_PRESENTERS_CHANGE_SCHEDULE_PRESENTER_H

#include "change_schedule_view.h"
#include "input_task.h"
#include "storage_task.h" /* For ConfigAccessTypeDef */
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ChangeSchedulePresenter ChangeSchedulePresenter_t;

ChangeSchedulePresenter_t* ChangeSchedulePresenter_Init(ChangeScheduleView_t *view, ConfigAccessTypeDef *config_access, bool skip_confirmation);
void ChangeSchedulePresenter_Deinit(ChangeSchedulePresenter_t *presenter);
void ChangeSchedulePresenter_HandleEvent(ChangeSchedulePresenter_t *presenter, const Input2VPEvent_t *event);
bool ChangeSchedulePresenter_IsComplete(ChangeSchedulePresenter_t *presenter);
bool ChangeSchedulePresenter_IsCancelled(ChangeSchedulePresenter_t *presenter);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_CHANGE_SCHEDULE_PRESENTER_H */
