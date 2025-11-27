/*
 * Input handling task that aggregates button events for other subsystems.
 */
#ifndef CORE_INC_INPUT_TASK_H
#define CORE_INC_INPUT_TASK_H

#include "buttons/buttons.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef enum
{
	EVT_MODE_BTN = 0,
	EVT_CENTRAL_BTN,
	EVT_MENU_BTN,
	EVT_CTRL_WHEEL_DELTA
} Input2VPEEventTypeDef;

typedef struct
{
	Input2VPEEventTypeDef type;
	button_action_t button_action;
	int16_t delta;
	uint32_t timestamp;
} Input2VPEEvent_t;

void StartInputTask(void *argument);

bool InputTask_TryGetEvent(Input2VPEEvent_t *event, uint32_t timeout_ticks);
bool InputTask_IsButtonPressed(button_id_t id);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_INPUT_TASK_H */