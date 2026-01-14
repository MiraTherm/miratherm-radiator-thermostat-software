#include "set_date_time_view.h"
#include <stdlib.h>

typedef struct SetDateTimeView {
  SetDateView_t *date_view;
  SetTimeView_t *time_view;
  SetBoolView_t *dst_view;
} SetDateTimeView_t;

SetDateTimeView_t *SetDateTimeView_Init(bool show_back_hint_on_first_field, uint16_t default_year) {
  SetDateTimeView_t *view =
      (SetDateTimeView_t *)malloc(sizeof(SetDateTimeView_t));
  if (!view)
    return NULL;

  view->date_view =
      SetDateView_Init("Set date:", show_back_hint_on_first_field, default_year);
  view->time_view = SetTimeView_Init("Set time:", true);
  view->dst_view = SetBoolView_Init("Summer time", "On", "Off", true);

  if (!view->date_view || !view->time_view || !view->dst_view) {
    SetDateTimeView_Deinit(view);
    return NULL;
  }

  return view;
}

void SetDateTimeView_Deinit(SetDateTimeView_t *view) {
  if (view) {
    if (view->date_view)
      SetDateView_Deinit(view->date_view);
    if (view->time_view)
      SetTimeView_Deinit(view->time_view);
    if (view->dst_view)
      SetBoolView_Deinit(view->dst_view);
    free(view);
  }
}

SetDateView_t *SetDateTimeView_GetDateView(SetDateTimeView_t *view) {
  return view ? view->date_view : NULL;
}

SetTimeView_t *SetDateTimeView_GetTimeView(SetDateTimeView_t *view) {
  return view ? view->time_view : NULL;
}

SetBoolView_t *SetDateTimeView_GetDstView(SetDateTimeView_t *view) {
  return view ? view->dst_view : NULL;
}
