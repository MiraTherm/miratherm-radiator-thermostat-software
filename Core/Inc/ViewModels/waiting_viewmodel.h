#ifndef CORE_INC_VIEWMODELS_WAITING_VIEWMODEL_H
#define CORE_INC_VIEWMODELS_WAITING_VIEWMODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Waiting view model
 */
typedef struct
{
    /* No dynamic data needed for now, just static text */
    uint8_t dummy; 
} Waiting_ViewModelData_t;

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWMODELS_WAITING_VIEWMODEL_H */
