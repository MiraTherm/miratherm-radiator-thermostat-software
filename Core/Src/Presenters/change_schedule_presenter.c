#include "change_schedule_presenter.h"
#include "change_schedule_view.h"
#include "set_bool_presenter.h"
#include "set_time_slot_presenter.h"
#include "set_value_presenter.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  STEP_ASK_CHANGE = 0,
  STEP_NUM_SLOTS,
  STEP_SLOT_TIME,
  STEP_SLOT_TEMP,
  STEP_FINISH
} ScheduleStep_t;

typedef struct ChangeSchedulePresenter {
  ChangeScheduleView_t *view;
  ConfigModel_t *config_model;

  SetBoolPresenter_t *bool_presenter;
  SetValuePresenter_t *value_presenter;
  SetTimeSlotPresenter_t *time_slot_presenter;

  ScheduleStep_t current_step;
  bool is_complete;
  bool is_cancelled;

  /* Temporary schedule data */
  DailyScheduleTypeDef schedule;
  uint8_t current_slot_index;

  /* For temperature roller options */
  char temp_options[512];

} ChangeSchedulePresenter_t;

static void load_schedule(ChangeSchedulePresenter_t *presenter);

ChangeSchedulePresenter_t *
ChangeSchedulePresenter_Init(ChangeScheduleView_t *view,
                             ConfigModel_t *config_model,
                             bool skip_confirmation) {
  ChangeSchedulePresenter_t *presenter =
      (ChangeSchedulePresenter_t *)malloc(sizeof(ChangeSchedulePresenter_t));
  if (!presenter)
    return NULL;

  presenter->view = view;
  presenter->config_model = config_model;
  presenter->current_step =
      skip_confirmation ? STEP_NUM_SLOTS : STEP_ASK_CHANGE;
  presenter->is_complete = false;
  presenter->is_cancelled = false;
  presenter->current_slot_index = 0;

  /* Initialize sub-presenters */
  presenter->bool_presenter =
      SetBoolPresenter_Init(ChangeScheduleView_GetBoolView(view));
  presenter->value_presenter =
      SetValuePresenter_Init(ChangeScheduleView_GetValueView(view), 0, 1);
  presenter->time_slot_presenter =
      SetTimeSlotPresenter_Init(ChangeScheduleView_GetTimeSlotView(view));

  if (!presenter->bool_presenter || !presenter->value_presenter ||
      !presenter->time_slot_presenter) {
    ChangeSchedulePresenter_Deinit(presenter);
    return NULL;
  }

  Utils_GenerateTempOptions(presenter->temp_options,
                            sizeof(presenter->temp_options));
  SetValueView_SetOptions(ChangeScheduleView_GetValueView(view),
                          presenter->temp_options);

  /* Load initial schedule from config */
  load_schedule(presenter);

  if (skip_confirmation) {
    /* Skip directly to asking number of slots */
    SetValueView_SetTitle(ChangeScheduleView_GetValueView(view),
                          "Num time slots");
    SetValueView_SetUnit(ChangeScheduleView_GetValueView(view), NULL);
    SetValueView_SetOptions(ChangeScheduleView_GetValueView(view), "3\n4\n5");
    SetValuePresenter_SetMaxIndex(presenter->value_presenter, 2);
    SetValuePresenter_SetSelectedIndex(presenter->value_presenter,
                                       presenter->schedule.num_time_slots - 3);
    SetValueView_Show(ChangeScheduleView_GetValueView(view));
  } else {
    /* Start with "Change schedule?" */
    SetBoolView_Show(ChangeScheduleView_GetBoolView(view));
  }

  return presenter;
}

void ChangeSchedulePresenter_Deinit(ChangeSchedulePresenter_t *presenter) {
  if (presenter) {
    if (presenter->bool_presenter)
      SetBoolPresenter_Deinit(presenter->bool_presenter);
    if (presenter->value_presenter)
      SetValuePresenter_Deinit(presenter->value_presenter);
    if (presenter->time_slot_presenter)
      SetTimeSlotPresenter_Deinit(presenter->time_slot_presenter);
    free(presenter);
  }
}

bool ChangeSchedulePresenter_IsCancelled(ChangeSchedulePresenter_t *presenter) {
  if (!presenter)
    return false;
  return presenter->is_cancelled;
}

static void save_schedule(ChangeSchedulePresenter_t *presenter) {
  if (!presenter || !presenter->config_model)
    return;

  if (osMutexAcquire(presenter->config_model->mutex, osWaitForever) == osOK) {
    presenter->config_model->data.daily_schedule = presenter->schedule;
    osMutexRelease(presenter->config_model->mutex);
  }
}

static bool is_time_less(uint8_t h1, uint8_t m1, uint8_t h2, uint8_t m2) {
  if (h1 < h2)
    return true;
  if (h1 == h2 && m1 < m2)
    return true;
  return false;
}

static bool is_schedule_valid(const DailyScheduleTypeDef *schedule) {
  if (schedule->num_time_slots < 3 || schedule->num_time_slots > 5)
    return false;

  /* First slot must start at 00:00 */
  if (schedule->time_slots[0].start_hour != 0 ||
      schedule->time_slots[0].start_minute != 0)
    return false;

  /* Last slot must end at 23:59 */
  if (schedule->time_slots[schedule->num_time_slots - 1].end_hour != 23 ||
      schedule->time_slots[schedule->num_time_slots - 1].end_minute != 59)
    return false;

  for (int i = 0; i < schedule->num_time_slots; i++) {
    /* Start < End */
    if (!is_time_less(schedule->time_slots[i].start_hour,
                      schedule->time_slots[i].start_minute,
                      schedule->time_slots[i].end_hour,
                      schedule->time_slots[i].end_minute)) {
      return false;
    }

    /* Contiguity */
    if (i > 0) {
      if (schedule->time_slots[i].start_hour !=
              schedule->time_slots[i - 1].end_hour ||
          schedule->time_slots[i].start_minute !=
              schedule->time_slots[i - 1].end_minute) {
        return false;
      }
    }
  }
  return true;
}


static void load_schedule(ChangeSchedulePresenter_t *presenter) {
  if (!presenter || !presenter->config_model)
    return;

  if (osMutexAcquire(presenter->config_model->mutex, osWaitForever) == osOK) {
    presenter->schedule = presenter->config_model->data.daily_schedule;
    osMutexRelease(presenter->config_model->mutex);
  }

  /* If invalid, load default */
  if (!is_schedule_valid(&presenter->schedule)) {
    Utils_LoadDefaultSchedule(&presenter->schedule, 3);
  }
}

static void setup_slot_time_view(ChangeSchedulePresenter_t *presenter) {
  char title[32];
  snprintf(title, sizeof(title),
           "Set %d/%d time slot:", presenter->current_slot_index + 1,
           presenter->schedule.num_time_slots);
  SetTimeSlotView_SetTitle(ChangeScheduleView_GetTimeSlotView(presenter->view),
                           title);

  SetTimeSlotViewData_t data = {0};

  data.start_hour =
      presenter->schedule.time_slots[presenter->current_slot_index].start_hour;
  data.start_minute =
      presenter->schedule.time_slots[presenter->current_slot_index].start_minute;
  data.end_hour =
      presenter->schedule.time_slots[presenter->current_slot_index].end_hour;
  data.end_minute =
      presenter->schedule.time_slots[presenter->current_slot_index].end_minute;

  /* Locks */
  if (presenter->current_slot_index == 0) {
    data.start_time_locked = true; /* Always 00:00 */
    data.start_hour = 0;
    data.start_minute = 0;
  }

  if (presenter->current_slot_index == presenter->schedule.num_time_slots - 1) {
    data.end_time_locked = true; /* Always 23:59 */
    data.end_hour = 23;
    data.end_minute = 59;
  }

  /* If not first slot, start time is end time of previous slot */
  if (presenter->current_slot_index > 0) {
    data.start_time_locked = true; /* Locked to previous end */
    data.start_hour =
        presenter->schedule.time_slots[presenter->current_slot_index - 1]
            .end_hour;
    data.start_minute =
        presenter->schedule.time_slots[presenter->current_slot_index - 1]
            .end_minute;
  }

  SetTimeSlotPresenter_SetData(presenter->time_slot_presenter, &data);
  SetTimeSlotPresenter_Reset(presenter->time_slot_presenter);
  SetTimeSlotView_Show(ChangeScheduleView_GetTimeSlotView(presenter->view));
}

static void setup_slot_temp_view(ChangeSchedulePresenter_t *presenter) {
  char title[32];
  snprintf(title, sizeof(title),
           "Set %d/%d temp:", presenter->current_slot_index + 1,
           presenter->schedule.num_time_slots);
  SetValueView_SetTitle(ChangeScheduleView_GetValueView(presenter->view),
                        title);
  SetValueView_SetUnit(ChangeScheduleView_GetValueView(presenter->view), "°C");

  /* Need to set options when coming from STEP_NUM_SLOTS which uses different
   * options */
  /* The SetValueView caches the options pointer, so this is only expensive on
   * first call */
  SetValueView_SetOptions(ChangeScheduleView_GetValueView(presenter->view),
                          presenter->temp_options);
  SetValueView_SetLeftButtonHint(
      ChangeScheduleView_GetValueView(presenter->view), true);
  SetValuePresenter_SetMaxIndex(presenter->value_presenter, 51); /* 0..51 */

  float current_temp =
      presenter->schedule.time_slots[presenter->current_slot_index].temperature;
  SetValuePresenter_SetSelectedIndex(presenter->value_presenter,
                                     Utils_TempToIndex(current_temp));

  SetValuePresenter_Reset(presenter->value_presenter);
  SetValueView_Show(ChangeScheduleView_GetValueView(presenter->view));
}

void ChangeSchedulePresenter_HandleEvent(ChangeSchedulePresenter_t *presenter,
                                         const Input2VPEvent_t *event) {
  if (!presenter || !event)
    return;

  switch (presenter->current_step) {
  case STEP_ASK_CHANGE:
    SetBoolPresenter_HandleEvent(presenter->bool_presenter, event);
    if (SetBoolPresenter_IsComplete(presenter->bool_presenter)) {
      const SetBoolViewData_t *data =
          SetBoolPresenter_GetData(presenter->bool_presenter);
      if (data->value) /* Yes */
      {
        /* Load current schedule to edit */
        load_schedule(presenter);

        /* Go to Num Slots selection */
        presenter->current_step = STEP_NUM_SLOTS;

        SetValueView_SetTitle(ChangeScheduleView_GetValueView(presenter->view),
                              "Time slots / day:");
        SetValueView_SetOptions(
            ChangeScheduleView_GetValueView(presenter->view), "3\n4\n5");
        SetValueView_SetLeftButtonHint(
            ChangeScheduleView_GetValueView(presenter->view), false);
        SetValuePresenter_SetMaxIndex(presenter->value_presenter,
                                      2); /* 0=3, 1=4, 2=5 */

        /* Map current num slots to index */
        uint16_t idx = 0;
        if (presenter->schedule.num_time_slots >= 3 &&
            presenter->schedule.num_time_slots <= 5)
          idx = presenter->schedule.num_time_slots - 3;
        SetValuePresenter_SetSelectedIndex(presenter->value_presenter, idx);

        SetValuePresenter_Reset(presenter->value_presenter);
        SetValueView_Show(ChangeScheduleView_GetValueView(presenter->view));
      } else /* No */
      {
        /* Skip changing */
        presenter->is_complete = true;
      }
    }
    break;

  case STEP_NUM_SLOTS:
    /* Handle Back */
    if (event->type == EVT_LEFT_BTN &&
        event->button_action == BUTTON_ACTION_PRESSED) {
      presenter->is_cancelled = true;
      return;
    }

    SetValuePresenter_HandleEvent(presenter->value_presenter, event);
    if (SetValuePresenter_IsComplete(presenter->value_presenter)) {
      uint16_t idx =
          SetValuePresenter_GetSelectedIndex(presenter->value_presenter);
      uint8_t new_num_slots = idx + 3;

      /* If num slots changed, use defaults */
      if (new_num_slots != presenter->schedule.num_time_slots) {
        Utils_LoadDefaultSchedule(&presenter->schedule, new_num_slots);
      }

      presenter->current_slot_index = 0;

      presenter->current_step = STEP_SLOT_TIME;
      setup_slot_time_view(presenter);
    }
    break;

  case STEP_SLOT_TIME:
    /* Handle Back */
    if (event->type == EVT_LEFT_BTN &&
        event->button_action == BUTTON_ACTION_PRESSED) {
      /* If we are at first field of time slot, go back to previous step */
      SetTimeSlotViewData_t data =
          SetTimeSlotPresenter_GetData(presenter->time_slot_presenter);

      /* If start time is locked, active field starts at 2. If not, 0. */
      bool at_start = (data.start_time_locked && data.active_field == 2) ||
                      (!data.start_time_locked && data.active_field == 0);

      if (at_start) {
        if (presenter->current_slot_index == 0) {
          /* Back to Num Slots */
          presenter->current_step = STEP_NUM_SLOTS;
          SetValueView_SetTitle(
              ChangeScheduleView_GetValueView(presenter->view),
              "Time slots / day:");
          SetValueView_SetOptions(
              ChangeScheduleView_GetValueView(presenter->view), "3\n4\n5");
          SetValueView_SetUnit(ChangeScheduleView_GetValueView(presenter->view),
                               "");
          SetValueView_SetLeftButtonHint(
              ChangeScheduleView_GetValueView(presenter->view), false);
          SetValuePresenter_SetMaxIndex(presenter->value_presenter, 2);
          SetValuePresenter_SetSelectedIndex(presenter->value_presenter,
                                            presenter->schedule.num_time_slots - 3);
          SetValuePresenter_Reset(presenter->value_presenter);
          SetValueView_Show(ChangeScheduleView_GetValueView(presenter->view));
        } else {
          /* Back to previous slot Temp */
          presenter->current_slot_index--;
          presenter->current_step = STEP_SLOT_TEMP;
          setup_slot_temp_view(presenter);
        }
        return;
      }
    }

    SetTimeSlotPresenter_HandleEvent(presenter->time_slot_presenter, event);
    if (SetTimeSlotPresenter_IsComplete(presenter->time_slot_presenter)) {
      /* Save time slot data */
      SetTimeSlotViewData_t data =
          SetTimeSlotPresenter_GetData(presenter->time_slot_presenter);

      /* Validate: end time must be greater than start time */
      bool is_valid_time_range = false;
      if (data.end_hour > data.start_hour) {
        is_valid_time_range = true;
      } else if (data.end_hour == data.start_hour &&
                 data.end_minute > data.start_minute) {
        is_valid_time_range = true;
      }

      if (!is_valid_time_range) {
        /* Invalid: end time is not greater than start time, reset the presenter
         */
        SetTimeSlotPresenter_Reset(presenter->time_slot_presenter);
        return;
      }

      presenter->schedule.time_slots[presenter->current_slot_index].start_hour =
          data.start_hour;
      presenter->schedule.time_slots[presenter->current_slot_index].start_minute =
          data.start_minute;
      presenter->schedule.time_slots[presenter->current_slot_index].end_hour =
          data.end_hour;
      presenter->schedule.time_slots[presenter->current_slot_index].end_minute =
          data.end_minute;

      /* Go to Temp */
      presenter->current_step = STEP_SLOT_TEMP;
      setup_slot_temp_view(presenter);
    }
    break;

  case STEP_SLOT_TEMP:
    /* Handle Back */
    if (event->type == EVT_LEFT_BTN &&
        event->button_action == BUTTON_ACTION_PRESSED) {
      /* Back to Time Slot */
      presenter->current_step = STEP_SLOT_TIME;
      setup_slot_time_view(presenter);
      return;
    }

    SetValuePresenter_HandleEvent(presenter->value_presenter, event);
    if (SetValuePresenter_IsComplete(presenter->value_presenter) &&
        !presenter->is_complete) {
      /* Save Temp */
      uint16_t idx =
          SetValuePresenter_GetSelectedIndex(presenter->value_presenter);
      presenter->schedule.time_slots[presenter->current_slot_index].temperature =
          Utils_IndexToTemp(idx);

      /* Next slot or Finish */
      if (presenter->current_slot_index <
          presenter->schedule.num_time_slots - 1) {
        presenter->current_slot_index++;
        presenter->current_step = STEP_SLOT_TIME;
        setup_slot_time_view(presenter);
      } else {
        /* Finished all slots */
        save_schedule(presenter);
        presenter->is_complete = true;
      }
    }
    break;

  default:
    break;
  }
}

bool ChangeSchedulePresenter_IsComplete(ChangeSchedulePresenter_t *presenter) {
  return presenter ? presenter->is_complete : false;
}
