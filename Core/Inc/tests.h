#ifndef TESTS_H
#define TESTS_H

#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DRIVER_TEST
void Driver_Test(osMessageQueueId_t storage2system_event_queue);
#elif ADAPTATION_TEST
void Adaptation_Test(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* TESTS_H */