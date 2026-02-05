#ifndef CORE_INC_PRESENTERS_BOOST_PRESENTER_H
#define CORE_INC_PRESENTERS_BOOST_PRESENTER_H

#include "boost_view.h"
#include "input_task.h"
#include "system_task.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BoostPresenter BoostPresenter_t;

BoostPresenter_t* BoostPresenter_Init(BoostView_t *view, SystemModel_t *system_model);
void BoostPresenter_Deinit(BoostPresenter_t *presenter);
void BoostPresenter_HandleEvent(BoostPresenter_t *presenter, const Input2VPEvent_t *event);
void BoostPresenter_Run(BoostPresenter_t *presenter, uint32_t current_tick);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_BOOST_PRESENTER_H */
