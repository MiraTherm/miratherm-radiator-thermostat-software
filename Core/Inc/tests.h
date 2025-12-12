#ifndef TESTS_H
#define TESTS_H

#include "cmsis_os2.h"
#include "storage_task.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DRIVER_TEST
void Driver_Test(osMessageQueueId_t storage2system_event_queue, ConfigAccessTypeDef *config_access);
#elif ADAPTATION_TEST
void Adaptation_Test(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* TESTS_H */