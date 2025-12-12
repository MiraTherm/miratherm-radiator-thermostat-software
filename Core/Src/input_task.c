#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "input_task.h"
#include "main.h"
#include "rotary_encoder.h"

#define INPUT_BUTTON_POLL_DELAY_MS 25U

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
#if INPUT_TASK_DEBUG_PRINTING
  printf("InputTask_PostEvent: type=%d action=%d delta=%d timestamp=%lu\n",
         (int)event->type,
         (int)event->button_action,
         (int)event->delta,
         (unsigned long)event->timestamp);
#endif

  (void)osMessageQueuePut(s_event_queue, event, 0U, 0U);
}

void StartInputTask(void *argument)
{
  const InputTaskArgsTypeDef *args = (const InputTaskArgsTypeDef *)argument;
  if (args == NULL)
  {
    Error_Handler();
  }

  s_event_queue = args->input2vp_event_queue;
  if (s_event_queue == NULL)
  {
    Error_Handler();
  }

#if OS_TASKS_DEBUG
  printf("InputTask running (heap=%lu)\n", (unsigned long)xPortGetFreeHeapSize());
#endif

  Buttons_Init();

  if (RotaryEncoder_Init() != HAL_OK)
  {
    Error_Handler();
  }

  button_event_t button_event;

  printf("InputTask init OK. Running loop...\n");

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

      if (button_event.id == BUTTON_ID_MIDDLE && button_event.action == BUTTON_ACTION_PRESSED)
      {
        static uint32_t last_middle_press = 0;
        if ((button_event.timestamp - last_middle_press) < 500) // 500ms double click threshold
        {
          event.type = EVT_CENTRAL_DOUBLE_CLICK;
          last_middle_press = 0; // Reset
        }
        else
        {
          last_middle_press = button_event.timestamp;
        }
      }

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

    osDelay(pdMS_TO_TICKS(INPUT_BUTTON_POLL_DELAY_MS));
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
#if INPUT_TASK_DEBUG_PRINTING
  printf("HAL_GPIO_EXTI_Callback: Pin=%u\n", GPIO_Pin);
#endif
  Buttons_HandleExtiCallback(GPIO_Pin);
}