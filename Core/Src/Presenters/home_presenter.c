#include "home_presenter.h"
#include "cmsis_os2.h"
#include "main.h"
#include "utils.h"
#include "view_presenter_router.h"
#include <stdio.h>
#include <stdlib.h>

extern RTC_HandleTypeDef hrtc;

struct HomePresenter {
  HomeView_t *view;
  SystemModel_t *system_model;
  ConfigModel_t *config_model;
  SensorModel_t *sensor_model;
};

HomePresenter_t *
HomePresenter_Init(HomeView_t *view, SystemModel_t *system_model,
                   ConfigModel_t *config_model,
                   SensorModel_t *sensor_model) {
  if (!view || !system_model || !config_model || !sensor_model)
    return NULL;

  HomePresenter_t *presenter =
      (HomePresenter_t *)malloc(sizeof(HomePresenter_t));
  if (!presenter)
    return NULL;

  presenter->view = view;
  presenter->system_model = system_model;
  presenter->config_model = config_model;
  presenter->sensor_model = sensor_model;

  return presenter;
}

void HomePresenter_Deinit(HomePresenter_t *presenter) {
  if (presenter) {
    free(presenter);
  }
}

void HomePresenter_HandleEvent(HomePresenter_t *presenter,
                               const Input2VPEvent_t *event) {
  if (!presenter || !event)
    return;

  /* Handle rotary encoder movement */
  if (event->type == EVT_CTRL_WHEEL_DELTA) {
    if (!presenter->system_model || !presenter->config_model)
      return;

    /* Check current mode */
    SystemMode_t current_mode = MODE_AUTO;
    if (osMutexAcquire(presenter->system_model->mutex, 10) == osOK) {
      current_mode = presenter->system_model->data.mode;
      osMutexRelease(presenter->system_model->mutex);
    }

    if (current_mode == MODE_AUTO) {
      /* AUTO mode: use temporary override */
      if (osMutexAcquire(presenter->system_model->mutex, 10) == osOK) {
        /* Use current temporary override if set, otherwise start from scheduled
         * target */
        float current_temp;
        if (presenter->system_model->data.temporary_target_temp != 0) {
          current_temp = presenter->system_model->data.temporary_target_temp;
        } else {
          current_temp = presenter->system_model->data.target_temp;
        }

        uint16_t current_index = Utils_TempToIndex(current_temp);

        /* Adjust index by delta (each encoder click = 1 step) */
        int16_t new_index = (int16_t)current_index + event->delta;
        if (new_index < 0)
          new_index = 0;
        if (new_index > 51)
          new_index = 51;

        float new_temp = Utils_IndexToTemp((uint16_t)new_index);
        presenter->system_model->data.temporary_target_temp = new_temp;

        printf("Home: AUTO mode - Rotary encoder delta=%d, new temp "
               "override=%.1f°C\n",
               event->delta, new_temp);

        osMutexRelease(presenter->system_model->mutex);
      }
    } else {
      /* MANUAL mode: adjust manual temperature directly */
      if (osMutexAcquire(presenter->config_model->mutex, 10) == osOK) {
        float current_temp = presenter->config_model->data.manual_target_temp;
        uint16_t current_index = Utils_TempToIndex(current_temp);

        /* Adjust index by delta (each encoder click = 1 step) */
        int16_t new_index = (int16_t)current_index + event->delta;
        if (new_index < 0)
          new_index = 0;
        if (new_index > 51)
          new_index = 51;

        float new_temp = Utils_IndexToTemp((uint16_t)new_index);
        presenter->config_model->data.manual_target_temp = new_temp;

        printf("Home: MANUAL mode - Rotary encoder delta=%d, new manual "
               "temp=%.1f°C\n",
               event->delta, new_temp);

        osMutexRelease(presenter->config_model->mutex);
      }
    }

    return;
  }

  /* Handle button presses */
  if (event->button_action == BUTTON_ACTION_PRESSED) {
    switch (event->type) {
    case EVT_LEFT_BTN:
      /* Toggle mode between AUTO and MANUAL */
      if (presenter->system_model) {
        if (osMutexAcquire(presenter->system_model->mutex, 10) == osOK) {
          /* Toggle mode */
          SystemMode_t new_mode =
              (presenter->system_model->data.mode == MODE_AUTO) ? MODE_MANUAL
                                                                  : MODE_AUTO;
          presenter->system_model->data.mode = new_mode;

          /* Clear temporary override when switching modes */
          presenter->system_model->data.temporary_target_temp = 0;

          printf("Home: Mode button pressed, switching to %s mode\n",
                 (new_mode == MODE_AUTO) ? "AUTO" : "MANUAL");

          osMutexRelease(presenter->system_model->mutex);
        }
      }
      break;
    case EVT_MIDDLE_BTN:
      /* Activate Boost mode */
      if (presenter->system_model) {
        if (osMutexAcquire(presenter->system_model->mutex, 10) == osOK) {
          /* Save current mode before boost */
          presenter->system_model->data.mode_before_boost =
              presenter->system_model->data.mode;
          /* Set boost mode and record start time */
          presenter->system_model->data.mode = MODE_BOOST;
          presenter->system_model->data.boost_begin_time =
              osKernelGetTickCount();

          printf("Home: Boost button pressed, entering boost mode\n");

          osMutexRelease(presenter->system_model->mutex);
        }
        /* Switch to boost view */
        Router_GoToRoute(ROUTE_BOOST);
      }
      break;
    case EVT_RIGHT_BTN:
      /* Handle Menu button */
      printf("Home: Menu button pressed\n");
      Router_GoToRoute(ROUTE_MENU);
      break;
    default:
      break;
    }
  }
}

void HomePresenter_Run(HomePresenter_t *presenter, uint32_t current_tick) {
  if (!presenter || !presenter->view)
    return;

  HomeViewData_t data = {0};

  /* Get Time */
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

  data.hour = sTime.Hours;
  data.minute = sTime.Minutes;

  /* Get Sensor Values */
  if (osMutexAcquire(presenter->sensor_model->mutex, 10) == osOK) {
    data.ambient_temperature = presenter->sensor_model->data.ambient_temperature;
    data.battery_percentage = presenter->sensor_model->data.soc;
    osMutexRelease(presenter->sensor_model->mutex);
  }

  /* Get Target Temperature and Mode from System State */
  if (osMutexAcquire(presenter->system_model->mutex, 10) == osOK) {
    data.target_temp = presenter->system_model->data.target_temp;
    data.mode = presenter->system_model->data.mode;

    /* Use temporary override if set, otherwise use scheduled target */
    if (presenter->system_model->data.temporary_target_temp != 0) {
      data.target_temp = presenter->system_model->data.temporary_target_temp;
    }

    data.slot_end_hour = presenter->system_model->data.slot_end_hour;
    data.slot_end_minute = presenter->system_model->data.slot_end_minute;

    /* Determine if displaying OFF or ON mode */
    data.is_off_mode = (data.target_temp <= 4.5f);
    data.is_on_mode = (data.target_temp >= 30.0f);

    osMutexRelease(presenter->system_model->mutex);
  }

  HomeView_Render(presenter->view, &data);
}
