#ifndef CORE_INC_VIEWS_SET_TIME_VIEW_H
#define CORE_INC_VIEWS_SET_TIME_VIEW_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef SetTimeViewData_t
 * @brief View data for time setting screen
 */
typedef struct
{
    uint8_t hour;     /* 0-23 */
    uint8_t minute;   /* 0-59 */
    uint8_t active_field;   /* 0: hour, 1: minute */
} SetTimeViewData_t;

typedef struct SetTimeView SetTimeView_t;

SetTimeView_t* SetTimeView_Init(const char *title, bool show_back_hint_on_first_field);
void SetTimeView_Deinit(SetTimeView_t *view);
void SetTimeView_Render(SetTimeView_t *view, const SetTimeViewData_t *data);
void SetTimeView_Show(SetTimeView_t *view);
void SetTimeView_Hide(SetTimeView_t *view);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_SET_TIME_VIEW_H */
