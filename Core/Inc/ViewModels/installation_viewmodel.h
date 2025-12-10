#ifndef CORE_INC_VIEWMODELS_INSTALLATION_VIEWMODEL_H
#define CORE_INC_VIEWMODELS_INSTALLATION_VIEWMODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Installation configuration view model
 */
typedef struct
{
    uint32_t progress; /* 0-100 or indefinite state */
} Installation_ViewModelData_t;

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWMODELS_INSTALLATION_VIEWMODEL_H */
