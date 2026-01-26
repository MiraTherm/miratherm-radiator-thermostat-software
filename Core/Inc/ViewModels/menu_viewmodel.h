#ifndef CORE_INC_VIEWMODELS_MENU_VIEWMODEL_H
#define CORE_INC_VIEWMODELS_MENU_VIEWMODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t selected_index;
    const char *options_str; /* Newline separated options */
} Menu_ViewModelData_t;

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWMODELS_MENU_VIEWMODEL_H */
