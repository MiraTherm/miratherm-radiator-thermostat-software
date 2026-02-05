#include "menu_presenter.h"
#include "view_presenter_router.h"
#include <stdio.h>
#include <stdlib.h>

struct MenuPresenter {
  MenuView_t *view;
  SystemModel_t *system_model;
  ConfigModel_t *config_model;
  SensorModel_t *sensor_model;

  uint16_t selected_index;
  const char *options;
  uint8_t num_options;
};

#define MENU_OPTION_SCHEDULE 0
#define MENU_OPTION_OFFSET 1
#define MENU_OPTION_FACTORY_RST 2

MenuPresenter_t *
MenuPresenter_Init(MenuView_t *view, SystemModel_t *system_model,
                   ConfigModel_t *config_model,
                   SensorModel_t *sensor_model) {
  if (!view || !system_model || !config_model || !sensor_model)
    return NULL;

  MenuPresenter_t *presenter =
      (MenuPresenter_t *)malloc(sizeof(MenuPresenter_t));
  if (!presenter)
    return NULL;

  presenter->view = view;
  presenter->system_model = system_model;
  presenter->config_model = config_model;
  presenter->sensor_model = sensor_model;

  presenter->selected_index = 0;
  presenter->options = "\n"; // Empty to save flash, we hardcode buttons in view
  presenter->num_options = 3;

  return presenter;
}

void MenuPresenter_Deinit(MenuPresenter_t *presenter) {
  if (presenter) {
    free(presenter);
  }
}

void MenuPresenter_HandleEvent(MenuPresenter_t *presenter,
                               const Input2VPEvent_t *event) {
  if (!presenter || !event)
    return;

  if (event->type == EVT_CTRL_WHEEL_DELTA) {
    if (event->delta > 0) {
      if (presenter->selected_index < presenter->num_options - 1) {
        presenter->selected_index++;
      }
      /* Stop at last option */
    } else if (event->delta < 0) {
      if (presenter->selected_index > 0) {
        presenter->selected_index--;
      }
      /* Stop at first option */
    }
  } else if (event->button_action == BUTTON_ACTION_PRESSED) {
    switch (event->type) {
    case EVT_LEFT_BTN:
      /* Left button: Back to Home */
      Router_GoToRoute(ROUTE_HOME);
      break;
    case EVT_MIDDLE_BTN:
      /* Select item */
      if (presenter->selected_index == MENU_OPTION_OFFSET) {
        Router_GoToRoute(ROUTE_EDIT_TEMP_OFFSET);
      } else if (presenter->selected_index == MENU_OPTION_SCHEDULE) {
        /* TODO: Go to Change Schedule */
        Router_GoToRoute(ROUTE_CHANGE_SCHEDULE);
      } else if (presenter->selected_index == MENU_OPTION_FACTORY_RST) {
        Router_GoToRoute(ROUTE_FACTORY_RESET);
      }
      break;
    default:
      break;
    }
  }
}

void MenuPresenter_Run(MenuPresenter_t *presenter, uint32_t current_tick) {
  if (!presenter || !presenter->view)
    return;

  MenuViewData_t data;
  data.selected_index = presenter->selected_index;
  data.options_str = presenter->options;

  MenuView_Render(presenter->view, &data);
}
