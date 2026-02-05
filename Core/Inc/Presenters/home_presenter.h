#ifndef CORE_INC_PRESENTERS_HOME_PRESENTER_H
#define CORE_INC_PRESENTERS_HOME_PRESENTER_H

#include "home_view.h"
#include "input_task.h"
#include "system_task.h"
#include "storage_task.h"
#include <stdbool.h>

#include "sensor_task.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HomePresenter HomePresenter_t;

HomePresenter_t* HomePresenter_Init(HomeView_t *view, SystemModel_t *system_context, ConfigModel_t *config_access, SensorModel_t *sensor_values_access);
void HomePresenter_Deinit(HomePresenter_t *presenter);
void HomePresenter_HandleEvent(HomePresenter_t *presenter, const Input2VPEvent_t *event);
void HomePresenter_Run(HomePresenter_t *presenter, uint32_t current_tick);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_HOME_PRESENTER_H */
