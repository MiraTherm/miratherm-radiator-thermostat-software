#ifndef CORE_INC_VIEWMODELS_BOOST_VIEWMODEL_H
#define CORE_INC_VIEWMODELS_BOOST_VIEWMODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t remaining_seconds;  /* Countdown from 300 to 0 */
} Boost_ViewModelData_t;

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWMODELS_BOOST_VIEWMODEL_H */
