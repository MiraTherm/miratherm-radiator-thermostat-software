#ifndef CORE_INC_VIEWMODELS_SET_VALUE_VIEWMODEL_H
#define CORE_INC_VIEWMODELS_SET_VALUE_VIEWMODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t selected_index;
    const char *options_str; /* LVGL roller options string (newline separated) */
} SetValue_ViewModelData_t;

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWMODELS_SET_VALUE_VIEWMODEL_H */
