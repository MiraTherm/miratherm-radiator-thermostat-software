#ifndef CORE_INC_VIEWS_SET_VALUE_VIEW_H
#define CORE_INC_VIEWS_SET_VALUE_VIEW_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef SetValueViewData_t
 * @brief View data for value setting screen
 */
typedef struct
{
    uint16_t selected_index;
    const char *options_str; /* LVGL roller options string (newline separated) */
} SetValueViewData_t;

typedef struct SetValueView SetValueView_t;

SetValueView_t* SetValueView_Init(const char *title, const char *unit, const char *options);
void SetValueView_Deinit(SetValueView_t *view);
void SetValueView_Render(SetValueView_t *view, const SetValueViewData_t *data);
void SetValueView_Show(SetValueView_t *view);
void SetValueView_Hide(SetValueView_t *view);
void SetValueView_SetTitle(SetValueView_t *view, const char *title);
void SetValueView_SetUnit(SetValueView_t *view, const char *unit);
void SetValueView_SetOptions(SetValueView_t *view, const char *options);
void SetValueView_SetLeftButtonHint(SetValueView_t *view, bool show);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_SET_VALUE_VIEW_H */
