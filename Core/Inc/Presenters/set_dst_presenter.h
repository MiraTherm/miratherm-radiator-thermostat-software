#ifndef CORE_INC_PRESENTERS_SET_DST_PRESENTER_H
#define CORE_INC_PRESENTERS_SET_DST_PRESENTER_H

#include "set_dst_viewmodel.h"
#include "input_task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetDstPresenter SetDstPresenter_t;
typedef struct SetDstView SetDstView_t;

SetDstPresenter_t* SetDstPresenter_Init(SetDstView_t *view);
void SetDstPresenter_Deinit(SetDstPresenter_t *presenter);
void SetDstPresenter_HandleEvent(SetDstPresenter_t *presenter, const Input2VPEvent_t *event);
void SetDstPresenter_Reset(SetDstPresenter_t *presenter);
bool SetDstPresenter_IsComplete(SetDstPresenter_t *presenter);
const SetDst_ViewModelData_t* SetDstPresenter_GetData(SetDstPresenter_t *presenter);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_SET_DST_PRESENTER_H */
