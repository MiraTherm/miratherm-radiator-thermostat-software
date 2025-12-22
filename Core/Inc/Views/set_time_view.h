#ifndef CORE_INC_VIEWS_SET_TIME_VIEW_H
#define CORE_INC_VIEWS_SET_TIME_VIEW_H

#include "set_time_viewmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetTimeView SetTimeView_t;

SetTimeView_t* SetTimeView_Init(void);
void SetTimeView_Deinit(SetTimeView_t *view);
void SetTimeView_Render(SetTimeView_t *view, const SetTime_ViewModelData_t *data);
void SetTimeView_Show(SetTimeView_t *view);
void SetTimeView_Hide(SetTimeView_t *view);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_SET_TIME_VIEW_H */
