/*
 * Input handling task that aggregates button events for other subsystems.
 */
#ifndef CORE_INC_INPUT_TASK_H
#define CORE_INC_INPUT_TASK_H

#include "buttons.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
  EVT_LEFT_BTN = 0,
  EVT_MIDDLE_BTN,
  EVT_RIGHT_BTN,
  EVT_CTRL_WHEEL_DELTA,
  EVT_MIDDLE_DOUBLE_CLICK
} Input2VPEventTypeDef;

typedef struct {
  Input2VPEventTypeDef type;
  button_action_t button_action;
  int16_t delta;
  uint32_t timestamp;
} Input2VPEvent_t;

typedef struct {
  osMessageQueueId_t input2vp_event_queue;
} InputTaskArgsTypeDef;

void StartInputTask(void *argument);

#define INPUT_TASK_STACK_SIZE (512U * 4U)

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_INPUT_TASK_H */