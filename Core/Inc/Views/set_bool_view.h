#ifndef CORE_INC_VIEWS_SET_BOOL_VIEW_H
#define CORE_INC_VIEWS_SET_BOOL_VIEW_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef SetBoolViewData_t
 * @brief View data for boolean setting screen
 */
typedef struct
{
    bool value; /* true/false */
} SetBoolViewData_t;

typedef struct SetBoolView SetBoolView_t;

SetBoolView_t* SetBoolView_Init(const char *title, const char *option_true, const char *option_false, bool show_back_hint);
void SetBoolView_Deinit(SetBoolView_t *view);
void SetBoolView_Render(SetBoolView_t *view, const SetBoolViewData_t *data);
void SetBoolView_Show(SetBoolView_t *view);
void SetBoolView_Hide(SetBoolView_t *view);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_SET_BOOL_VIEW_H */
