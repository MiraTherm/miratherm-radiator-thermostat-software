#include "set_date_time_presenter.h"
#include "set_bool_presenter.h"
#include "set_date_presenter.h"
#include "set_time_presenter.h"
#include "stm32wbxx_hal.h"
#include <stdlib.h>

extern RTC_HandleTypeDef hrtc;

typedef struct SetDateTimePresenter {
  SetDateTimeView_t *view;

  SetDatePresenter_t *date_presenter;
  SetTimePresenter_t *time_presenter;
  SetBoolPresenter_t *dst_presenter;

  uint8_t current_step; /* 0: Date, 1: Time, 2: DST */
  bool is_complete;
} SetDateTimePresenter_t;

static void set_rtc(SetDateTimePresenter_t *presenter) {
  if (!presenter)
    return;
  if (hrtc.Instance == NULL)
    return;

  const SetDate_ViewModelData_t *date_data =
      SetDatePresenter_GetData(presenter->date_presenter);
  const SetTime_ViewModelData_t *time_data =
      SetTimePresenter_GetData(presenter->time_presenter);
  const SetBool_ViewModelData_t *dst_data =
      SetBoolPresenter_GetData(presenter->dst_presenter);

  if (!date_data || !time_data || !dst_data)
    return;

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  sDate.Year = (uint8_t)(date_data->year - 2000);
  sDate.Month = date_data->month;
  sDate.Date = date_data->day;
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;

  sTime.Hours = time_data->hour;
  sTime.Minutes = time_data->minute;
  sTime.Seconds = 0;
  sTime.TimeFormat = RTC_HOURFORMAT_24;
  sTime.DayLightSaving =
      (dst_data->value) ? RTC_DAYLIGHTSAVING_ADD1H : RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;

  HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

SetDateTimePresenter_t *SetDateTimePresenter_Init(SetDateTimeView_t *view, uint16_t default_year) {
  SetDateTimePresenter_t *presenter =
      (SetDateTimePresenter_t *)malloc(sizeof(SetDateTimePresenter_t));
  if (!presenter)
    return NULL;

  presenter->view = view;
  presenter->current_step = 0;
  presenter->is_complete = false;

  presenter->date_presenter =
      SetDatePresenter_Init(SetDateTimeView_GetDateView(view), default_year);
  presenter->time_presenter =
      SetTimePresenter_Init(SetDateTimeView_GetTimeView(view));
  presenter->dst_presenter =
      SetBoolPresenter_Init(SetDateTimeView_GetDstView(view));

  if (!presenter->date_presenter || !presenter->time_presenter ||
      !presenter->dst_presenter) {
    SetDateTimePresenter_Deinit(presenter);
    return NULL;
  }

  /* Show initial view */
  SetDateView_Show(SetDateTimeView_GetDateView(view));
  /* Force render initial state */
  SetDatePresenter_HandleEvent(presenter->date_presenter, NULL);

  return presenter;
}

void SetDateTimePresenter_Deinit(SetDateTimePresenter_t *presenter) {
  if (presenter) {
    if (presenter->date_presenter)
      SetDatePresenter_Deinit(presenter->date_presenter);
    if (presenter->time_presenter)
      SetTimePresenter_Deinit(presenter->time_presenter);
    if (presenter->dst_presenter)
      SetBoolPresenter_Deinit(presenter->dst_presenter);
    free(presenter);
  }
}

void SetDateTimePresenter_HandleEvent(SetDateTimePresenter_t *presenter,
                                      const Input2VPEvent_t *event) {
  if (!presenter || !event)
    return;

  if (presenter->current_step == 0) {
    SetDatePresenter_HandleEvent(presenter->date_presenter, event);
    if (SetDatePresenter_IsComplete(presenter->date_presenter)) {
      presenter->current_step = 1;
      SetTimeView_Show(SetDateTimeView_GetTimeView(presenter->view));
      /* Force render */
      SetTimePresenter_HandleEvent(presenter->time_presenter, NULL);
    }
  } else if (presenter->current_step == 1) {
    if (event->type == EVT_LEFT_BTN &&
        event->button_action == BUTTON_ACTION_PRESSED) {
      const SetTime_ViewModelData_t *data =
          SetTimePresenter_GetData(presenter->time_presenter);
      if (data->active_field == 0) {
        presenter->current_step = 0;
        SetDatePresenter_Reset(presenter->date_presenter);
        SetDateView_Show(SetDateTimeView_GetDateView(presenter->view));
        /* Force render */
        SetDatePresenter_HandleEvent(presenter->date_presenter, NULL);
        return;
      }
    }

    SetTimePresenter_HandleEvent(presenter->time_presenter, event);
    if (SetTimePresenter_IsComplete(presenter->time_presenter)) {
      presenter->current_step = 2;
      SetBoolView_Show(SetDateTimeView_GetDstView(presenter->view));
      /* Force render */
      SetBoolPresenter_HandleEvent(presenter->dst_presenter, NULL);
    }
  } else if (presenter->current_step == 2) {
    if (event->type == EVT_LEFT_BTN &&
        event->button_action == BUTTON_ACTION_PRESSED) {
      presenter->current_step = 1;
      SetTimePresenter_Reset(presenter->time_presenter);
      SetTimeView_Show(SetDateTimeView_GetTimeView(presenter->view));
      /* Force render */
      SetTimePresenter_HandleEvent(presenter->time_presenter, NULL);
      return;
    }

    SetBoolPresenter_HandleEvent(presenter->dst_presenter, event);
    if (SetBoolPresenter_IsComplete(presenter->dst_presenter)) {
      set_rtc(presenter);
      presenter->is_complete = true;
    }
  }
}

bool SetDateTimePresenter_IsComplete(SetDateTimePresenter_t *presenter) {
  return presenter ? presenter->is_complete : false;
}
