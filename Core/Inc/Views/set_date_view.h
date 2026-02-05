#ifndef CORE_INC_VIEWS_SET_DATE_VIEW_H
#define CORE_INC_VIEWS_SET_DATE_VIEW_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef SetDateViewData_t
 * @brief View data for date setting screen
 */
typedef struct
{
    uint8_t day;      /* 1-31 */
    uint8_t month;    /* 1-12 */
    uint16_t year;    /* e.g., 2026 */
    uint8_t active_field;   /* 0: year, 1: month, 2: day */
} SetDateViewData_t;

typedef struct SetDateView SetDateView_t;

SetDateView_t* SetDateView_Init(const char *title, bool show_back_hint_on_first_field, uint16_t default_year);
void SetDateView_Deinit(SetDateView_t *view);
void SetDateView_Render(SetDateView_t *view, const SetDateViewData_t *data);
void SetDateView_Show(SetDateView_t *view);
void SetDateView_Hide(SetDateView_t *view);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_SET_DATE_VIEW_H */
