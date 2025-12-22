#ifndef CORE_INC_VIEWS_SET_DST_VIEW_H
#define CORE_INC_VIEWS_SET_DST_VIEW_H

#include "set_dst_viewmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetDstView SetDstView_t;

SetDstView_t* SetDstView_Init(void);
void SetDstView_Deinit(SetDstView_t *view);
void SetDstView_Render(SetDstView_t *view, const SetDst_ViewModelData_t *data);
void SetDstView_Show(SetDstView_t *view);
void SetDstView_Hide(SetDstView_t *view);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_SET_DST_VIEW_H */
