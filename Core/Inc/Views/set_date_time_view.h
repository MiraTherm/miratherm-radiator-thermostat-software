#ifndef CORE_INC_VIEWS_SET_DATE_TIME_VIEW_H
#define CORE_INC_VIEWS_SET_DATE_TIME_VIEW_H

#include "set_date_view.h"
#include "set_time_view.h"
#include "set_bool_view.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetDateTimeView SetDateTimeView_t;

SetDateTimeView_t* SetDateTimeView_Init(bool show_back_hint_on_first_field, uint16_t default_year);
void SetDateTimeView_Deinit(SetDateTimeView_t *view);

SetDateView_t* SetDateTimeView_GetDateView(SetDateTimeView_t *view);
SetTimeView_t* SetDateTimeView_GetTimeView(SetDateTimeView_t *view);
SetBoolView_t* SetDateTimeView_GetDstView(SetDateTimeView_t *view);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_SET_DATE_TIME_VIEW_H */
