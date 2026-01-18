#include "set_temp_offset_presenter.h"
#include "set_value_presenter.h"
#include <stdio.h>
#include <stdlib.h>

struct SetTempOffsetPresenter {
  SetValuePresenter_t *generic_presenter;
  ConfigAccessTypeDef *config_access;
  char *options_str;
  bool is_complete;
  bool is_cancelled;
};

SetTempOffsetPresenter_t *
SetTempOffsetPresenter_Init(SetValueView_t *view,
                            ConfigAccessTypeDef *config_access) {
  if (!view || !config_access)
    return NULL;

  SetTempOffsetPresenter_t *presenter =
      (SetTempOffsetPresenter_t *)malloc(sizeof(SetTempOffsetPresenter_t));
  if (!presenter)
    return NULL;

  presenter->config_access = config_access;
  presenter->is_complete = false;
  presenter->is_cancelled = false;
  presenter->options_str = NULL;

  /* Generate Options String */
  /* -15.0 to +15.0, 0.5 steps. 61 items. */
  presenter->options_str = (char *)malloc(512);
  if (!presenter->options_str) {
    free(presenter);
    return NULL;
  }

  char *ptr = presenter->options_str;
  size_t remaining = 512;
  for (int i = 0; i <= 60; i++) {
    float val = -15.0f + (float)i * 0.5f;
    int len = 0;
    if (val > 0.001f) {
      len = snprintf(ptr, remaining, "+%.1f", val);
    } else if (val < -0.001f) {
      len = snprintf(ptr, remaining, "%.1f", val);
    } else {
      len = snprintf(ptr, remaining, "0.0");
    }
    if (len > 0 && (size_t)len < remaining) {
      ptr += len;
      remaining -= len;
      if (i < 60) {
        if (remaining > 1) {
          *ptr = '\n';
          ptr++;
          remaining--;
          *ptr = '\0';
        }
      }
    }
  }

  /* Configure View */
  SetValueView_SetTitle(view, "Temp Offset");
  SetValueView_SetUnit(view, "°C");
  SetValueView_SetOptions(view, presenter->options_str);
  SetValueView_Show(view);

  /* Calculate initial index */
  uint16_t initial_index = 30; /* Default 0.0 */
  if (osMutexAcquire(config_access->mutex, 10) == osOK) {
    float current_offset = config_access->data.TemperatureOffsetC;
    /* Calculate index: (offset + 15.0) / 0.5 */
    int idx = (int)((current_offset + 15.0f) * 2.0f);
    if (idx < 0)
      idx = 0;
    if (idx > 60)
      idx = 60;
    initial_index = (uint16_t)idx;
    osMutexRelease(config_access->mutex);
  }

  /* Initialize Generic Presenter */
  presenter->generic_presenter =
      SetValuePresenter_Init(view, initial_index, 60);
  if (!presenter->generic_presenter) {
    free(presenter->options_str);
    free(presenter);
    return NULL;
  }

  /* Force update view selection */
  SetValuePresenter_SetSelectedIndex(presenter->generic_presenter,
                                     initial_index);

  return presenter;
}

void SetTempOffsetPresenter_Deinit(SetTempOffsetPresenter_t *presenter) {
  if (presenter) {
    if (presenter->generic_presenter) {
      SetValuePresenter_Deinit(presenter->generic_presenter);
    }
    if (presenter->options_str) {
      free(presenter->options_str);
    }
    free(presenter);
  }
}

void SetTempOffsetPresenter_HandleEvent(SetTempOffsetPresenter_t *presenter,
                                        const Input2VPEvent_t *event) {
  if (!presenter || !presenter->generic_presenter)
    return;

  if (event->type == EVT_LEFT_BTN &&
      event->button_action == BUTTON_ACTION_PRESSED) {
    presenter->is_cancelled = true;
    return;
  }

  SetValuePresenter_HandleEvent(presenter->generic_presenter, event);

  if (SetValuePresenter_IsComplete(presenter->generic_presenter) &&
      !presenter->is_complete) {
    /* Save value */
    uint16_t index =
        SetValuePresenter_GetSelectedIndex(presenter->generic_presenter);
    /* Map index to offset. Range -15.0 to +15.0 with 0.5 step. */
    float new_offset = (float)index * 0.5f - 15.0f;

    if (osMutexAcquire(presenter->config_access->mutex, 10) == osOK) {
      presenter->config_access->data.TemperatureOffsetC = new_offset;
      osMutexRelease(presenter->config_access->mutex);
    }

    presenter->is_complete = true;
  }
}

bool SetTempOffsetPresenter_IsComplete(SetTempOffsetPresenter_t *presenter) {
  if (!presenter)
    return false;
  return presenter->is_complete;
}

bool SetTempOffsetPresenter_IsCancelled(SetTempOffsetPresenter_t *presenter) {
  if (!presenter)
    return false;
  return presenter->is_cancelled;
}
