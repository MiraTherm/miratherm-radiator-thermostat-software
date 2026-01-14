#ifndef CORE_INC_VIEWS_SET_DATE_VIEW_H
#define CORE_INC_VIEWS_SET_DATE_VIEW_H

#include <stdbool.h>
#include "set_date_viewmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SetDateView SetDateView_t;

SetDateView_t* SetDateView_Init(const char *title, bool show_back_hint_on_first_field, uint16_t default_year);
void SetDateView_Deinit(SetDateView_t *view);
void SetDateView_Render(SetDateView_t *view, const SetDate_ViewModelData_t *data);
void SetDateView_Show(SetDateView_t *view);
void SetDateView_Hide(SetDateView_t *view);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_SET_DATE_VIEW_H */
