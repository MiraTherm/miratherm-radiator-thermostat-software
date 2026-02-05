#ifndef CORE_INC_PRESENTERS_MENU_PRESENTER_H
#define CORE_INC_PRESENTERS_MENU_PRESENTER_H

#include "menu_view.h"
#include "input_task.h"
#include "system_task.h"
#include "storage_task.h"
#include "sensor_task.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MenuPresenter MenuPresenter_t;

MenuPresenter_t* MenuPresenter_Init(MenuView_t *view, SystemModel_t *system_context, ConfigModel_t *config_access, SensorModel_t *sensor_values_access);
void MenuPresenter_Deinit(MenuPresenter_t *presenter);
void MenuPresenter_HandleEvent(MenuPresenter_t *presenter, const Input2VPEvent_t *event);
void MenuPresenter_Run(MenuPresenter_t *presenter, uint32_t current_tick);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_MENU_PRESENTER_H */
