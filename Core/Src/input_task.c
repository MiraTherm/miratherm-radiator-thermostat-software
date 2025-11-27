#include "cmsis_os2.h"
#include "input_task.h"
#include "main.h"
#include "rotary_encoder.h"

#define INPUT_BUTTON_QUEUE_DEPTH 8U
#define INPUT_BUTTON_POLL_DELAY_MS 5U

static osMessageQueueId_t s_event_queue;

static Input2VPEventTypeDef ButtonToVP(button_id_t id)
{
  switch (id)
  {
    case BUTTON_ID_LEFT:
      return EVT_MODE_BTN;
    case BUTTON_ID_MIDDLE:
      return EVT_CENTRAL_BTN;
    case BUTTON_ID_RIGHT:
      return EVT_MENU_BTN;
    default:
      return EVT_CENTRAL_BTN;
  }
}

static void InputTask_PostEvent(const Input2VPEvent_t *event)
{
  if ((event == NULL) || (s_event_queue == NULL))
  {
    return;
  }

  (void)osMessageQueuePut(s_event_queue, event, 0U, 0U);
}

void StartInputTask(void *argument)
{
  (void)argument;

  s_event_queue = osMessageQueueNew(INPUT_BUTTON_QUEUE_DEPTH, sizeof(Input2VPEvent_t), NULL);
  if (s_event_queue == NULL)
  {
    Error_Handler();
  }

  if (RotaryEncoder_Init() != HAL_OK)
  {
    Error_Handler();
  }

  button_event_t button_event;

  for (;;)
  {
    while (Buttons_Poll(&button_event))
    {
      Input2VPEvent_t event = {
        .type = ButtonToVP(button_event.id),
        .button_action = button_event.action,
        .delta = 0,
        .timestamp = button_event.timestamp,
      };

      InputTask_PostEvent(&event);
    }

    const int8_t delta = RotaryEncoder_GetDelta();
    if (delta != 0)
    {
      Input2VPEvent_t event = {
        .type = EVT_CTRL_WHEEL_DELTA,
        .button_action = BUTTON_ACTION_RELEASED,
        .delta = delta,
        .timestamp = HAL_GetTick(),
      };

      InputTask_PostEvent(&event);
    }

    osDelay(INPUT_BUTTON_POLL_DELAY_MS);
  }
}

bool InputTask_TryGetEvent(Input2VPEvent_t *event, uint32_t timeout_ticks)
{
  if ((event == NULL) || (s_event_queue == NULL))
  {
    return false;
  }

  return (osMessageQueueGet(s_event_queue, event, NULL, timeout_ticks) == osOK);
}

bool InputTask_IsButtonPressed(button_id_t id)
{
  return Buttons_GetStableState(id);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  Buttons_HandleExtiCallback(GPIO_Pin);
}