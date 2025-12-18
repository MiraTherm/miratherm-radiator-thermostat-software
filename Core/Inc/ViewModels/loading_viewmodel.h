#ifndef CORE_INC_VIEWMODELS_LOADING_VIEWMODEL_H
#define CORE_INC_VIEWMODELS_LOADING_VIEWMODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Loading view model
 */
typedef struct
{
    uint32_t progress; /* 0-100 or indefinite state */
    uint32_t animation_frame; /* Animation frame counter */
} Loading_ViewModelData_t;

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWMODELS_LOADING_VIEWMODEL_H */
