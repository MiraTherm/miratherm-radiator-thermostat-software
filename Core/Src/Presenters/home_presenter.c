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
  SystemContextAccessTypeDef *system_context;
  ConfigAccessTypeDef *config_access;
  SensorValuesAccessTypeDef *sensor_values_access;
};

HomePresenter_t *
HomePresenter_Init(HomeView_t *view, SystemContextAccessTypeDef *system_context,
                   ConfigAccessTypeDef *config_access,
                   SensorValuesAccessTypeDef *sensor_values_access) {
  if (!view || !system_context || !config_access || !sensor_values_access)
    return NULL;

  HomePresenter_t *presenter =
      (HomePresenter_t *)malloc(sizeof(HomePresenter_t));
  if (!presenter)
    return NULL;

  presenter->view = view;
  presenter->system_context = system_context;
  presenter->config_access = config_access;
  presenter->sensor_values_access = sensor_values_access;

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
    if (!presenter->system_context || !presenter->config_access)
      return;

    /* Check current mode */
    SystemMode_t current_mode = MODE_AUTO;
    if (osMutexAcquire(presenter->system_context->mutex, 10) == osOK) {
      current_mode = presenter->system_context->data.mode;
      osMutexRelease(presenter->system_context->mutex);
    }

    if (current_mode == MODE_AUTO) {
      /* AUTO mode: use temporary override */
      if (osMutexAcquire(presenter->system_context->mutex, 10) == osOK) {
        /* Use current temporary override if set, otherwise start from scheduled
         * target */
        float current_temp;
        if (presenter->system_context->data.temporary_target_temp != 0) {
          current_temp = presenter->system_context->data.temporary_target_temp;
        } else {
          current_temp = presenter->system_context->data.target_temp;
        }

        uint16_t current_index = Utils_TempToIndex(current_temp);

        /* Adjust index by delta (each encoder click = 1 step) */
        int16_t new_index = (int16_t)current_index + event->delta;
        if (new_index < 0)
          new_index = 0;
        if (new_index > 51)
          new_index = 51;

        float new_temp = Utils_IndexToTemp((uint16_t)new_index);
        presenter->system_context->data.temporary_target_temp = new_temp;

        printf("Home: AUTO mode - Rotary encoder delta=%d, new temp "
               "override=%.1f°C\n",
               event->delta, new_temp);

        osMutexRelease(presenter->system_context->mutex);
      }
    } else {
      /* MANUAL mode: adjust manual temperature directly */
      if (osMutexAcquire(presenter->config_access->mutex, 10) == osOK) {
        float current_temp = presenter->config_access->data.ManualTargetTemp;
        uint16_t current_index = Utils_TempToIndex(current_temp);

        /* Adjust index by delta (each encoder click = 1 step) */
        int16_t new_index = (int16_t)current_index + event->delta;
        if (new_index < 0)
          new_index = 0;
        if (new_index > 51)
          new_index = 51;

        float new_temp = Utils_IndexToTemp((uint16_t)new_index);
        presenter->config_access->data.ManualTargetTemp = new_temp;

        printf("Home: MANUAL mode - Rotary encoder delta=%d, new manual "
               "temp=%.1f°C\n",
               event->delta, new_temp);

        osMutexRelease(presenter->config_access->mutex);
      }
    }

    return;
  }

  /* Handle button presses */
  if (event->button_action == BUTTON_ACTION_PRESSED) {
    switch (event->type) {
    case EVT_LEFT_BTN:
      /* Toggle mode between AUTO and MANUAL */
      if (presenter->system_context) {
        if (osMutexAcquire(presenter->system_context->mutex, 10) == osOK) {
          /* Toggle mode */
          SystemMode_t new_mode =
              (presenter->system_context->data.mode == MODE_AUTO) ? MODE_MANUAL
                                                                  : MODE_AUTO;
          presenter->system_context->data.mode = new_mode;

          /* Clear temporary override when switching modes */
          presenter->system_context->data.temporary_target_temp = 0;

          printf("Home: Mode button pressed, switching to %s mode\n",
                 (new_mode == MODE_AUTO) ? "AUTO" : "MANUAL");

          osMutexRelease(presenter->system_context->mutex);
        }
      }
      break;
    case EVT_MIDDLE_BTN:
      /* Activate Boost mode */
      if (presenter->system_context) {
        if (osMutexAcquire(presenter->system_context->mutex, 10) == osOK) {
          /* Save current mode before boost */
          presenter->system_context->data.mode_before_boost =
              presenter->system_context->data.mode;
          /* Set boost mode and record start time */
          presenter->system_context->data.mode = MODE_BOOST;
          presenter->system_context->data.boost_begin_time =
              osKernelGetTickCount();

          printf("Home: Boost button pressed, entering boost mode\n");

          osMutexRelease(presenter->system_context->mutex);
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

  HomeViewModel_t model = {0};

  /* Get Time */
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

  model.hour = sTime.Hours;
  model.minute = sTime.Minutes;

  /* Get Sensor Values */
  if (osMutexAcquire(presenter->sensor_values_access->mutex, 10) == osOK) {
    model.current_temp = presenter->sensor_values_access->data.CurrentTemp;
    model.battery_percentage = presenter->sensor_values_access->data.SoC;
    osMutexRelease(presenter->sensor_values_access->mutex);
  }

  /* Get Target Temperature and Mode from System State */
  if (osMutexAcquire(presenter->system_context->mutex, 10) == osOK) {
    model.target_temp = presenter->system_context->data.target_temp;
    model.mode = presenter->system_context->data.mode;

    /* Use temporary override if set, otherwise use scheduled target */
    if (presenter->system_context->data.temporary_target_temp != 0) {
      model.target_temp = presenter->system_context->data.temporary_target_temp;
    }

    model.slot_end_hour = presenter->system_context->data.slot_end_hour;
    model.slot_end_minute = presenter->system_context->data.slot_end_minute;

    /* Determine if displaying OFF or ON mode */
    model.is_off_mode = (model.target_temp <= 4.5f);
    model.is_on_mode = (model.target_temp >= 30.0f);

    osMutexRelease(presenter->system_context->mutex);
  }

  HomeView_Render(presenter->view, &model);
}
