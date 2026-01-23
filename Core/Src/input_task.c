/**
 ******************************************************************************
 * @file           :  input_task.c
 * @brief          :  Implementation of input event aggregation task
 *
 * @details        :  Implements continuous polling of buttons and rotary
 *                    encoder, converting physical state changes into
 *                    asynchronous events posted to view presenter task.
 *                    Handles GPIO interrupts for input acceleration and
 *                    supports debug output of input events.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#include "input_task.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "main.h"
#include "rotary_encoder.h"

/* Button polling interval in milliseconds */
#define INPUT_BUTTON_POLL_DELAY_MS 25U

/* Global message queue handle for event posting */
static osMessageQueueId_t s_event_queue;

/**
 * Map button ID to input event type for view presenter.
 * Converts button_id_t (LEFT/MIDDLE/RIGHT) to Input2VPEventTypeDef.
 */
static Input2VPEventTypeDef ButtonToVP(button_id_t id) {
  switch (id) {
  case BUTTON_ID_LEFT:
    return EVT_LEFT_BTN;
  case BUTTON_ID_MIDDLE:
    return EVT_MIDDLE_BTN;
  case BUTTON_ID_RIGHT:
    return EVT_RIGHT_BTN;
  default:
    return EVT_MIDDLE_BTN; /* Fallback to middle button */
  }
}

/**
 * Post input event to view presenter message queue.
 * Validates queue handle and event pointer before posting.
 * Optional debug output shows event type, action, delta, and timestamp.
 */
static void InputTask_PostEvent(const Input2VPEvent_t *event) {
  if ((event == NULL) || (s_event_queue == NULL)) {
    return;
  }
#if INPUT_TASK_DEBUG_PRINTING
  printf("InputTask_PostEvent: type=%d action=%d delta=%d timestamp=%lu\n",
         (int)event->type, (int)event->button_action, (int)event->delta,
         (unsigned long)event->timestamp);
#endif

  /* Post event to queue without timeout (non-blocking) */
  (void)osMessageQueuePut(s_event_queue, event, 0U, 0U);
}

void StartInputTask(void *argument) {
  const InputTaskArgsTypeDef *args = (const InputTaskArgsTypeDef *)argument;
  if (args == NULL) {
    Error_Handler();
  }

  /* Cache message queue handle for event posting */
  s_event_queue = args->input2vp_event_queue;
  if (s_event_queue == NULL) {
    Error_Handler();
  }

#if OS_TASKS_DEBUG
  printf("InputTask running (heap=%lu)\n",
         (unsigned long)xPortGetFreeHeapSize());
#endif

  /* Initialize button driver with GPIO setup and debounce timers */
  Buttons_Init();

  /* Initialize rotary encoder with TIM2 encoder mode */
  if (RotaryEncoder_Init() != HAL_OK) {
    Error_Handler();
  }

  button_event_t button_event;

  printf("InputTask init OK. Running loop...\n");

  /* Main polling loop: 25ms cycle time */
  for (;;) {
    /* Poll buttons until all pending events are processed */
    while (Buttons_Poll(&button_event)) {
      Input2VPEvent_t event = {
          .type = ButtonToVP(button_event.id),
          .button_action = button_event.action,
          .delta = 0, /* Not applicable for button events */
          .timestamp = button_event.timestamp,
      };

      InputTask_PostEvent(&event);
    }

    /* Check for rotary encoder rotation since last poll */
    const int8_t delta = RotaryEncoder_GetDelta();
    if (delta != 0) {
      Input2VPEvent_t event = {
          .type = EVT_CTRL_WHEEL_DELTA,
          .button_action = BUTTON_ACTION_RELEASED, /* N/A for encoder */
          .delta = delta, /* Positive=clockwise, negative=counter-clockwise */
          .timestamp = HAL_GetTick(),
      };

      InputTask_PostEvent(&event);
    }

    /* Sleep until next polling cycle (25ms interval) */
    osDelay(pdMS_TO_TICKS(INPUT_BUTTON_POLL_DELAY_MS));
  }
}

/**
 * GPIO interrupt callback for button state changes.
 * Called by HAL when GPIO interrupt fires. Delegates to button driver's
 * interrupt handler to mark button state as needing re-poll.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
#if INPUT_TASK_DEBUG_PRINTING
  printf("HAL_GPIO_EXTI_Callback: Pin=%u\n", GPIO_Pin);
#endif
  Buttons_HandleExtiCallback(GPIO_Pin);
}