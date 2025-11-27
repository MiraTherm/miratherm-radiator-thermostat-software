/*
 * Debounced button driver using edge timestamps and the system tick counter.
 */
#ifndef DRIVERS_BUTTONS_H
#define DRIVERS_BUTTONS_H

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

typedef enum
{
  BUTTON_ID_MIDDLE = 0,
  BUTTON_ID_LEFT,
  BUTTON_ID_RIGHT,
  BUTTON_ID_COUNT
} button_id_t;

typedef enum
{
  BUTTON_ACTION_RELEASED = 0,
  BUTTON_ACTION_PRESSED
} button_action_t;

typedef struct
{
  button_id_t id;
  button_action_t action;
  uint32_t timestamp;
} button_event_t;

void Buttons_Init(void);
void Buttons_RecordEdge(button_id_t id);
bool Buttons_Poll(button_event_t *event);
bool Buttons_GetStableState(button_id_t id);

static inline void Buttons_HandleExtiCallback(uint16_t gpio_pin)
{
  switch (gpio_pin)
  {
    case BUTTON_MIDDLE_Pin:
      Buttons_RecordEdge(BUTTON_ID_MIDDLE);
      break;
    case BUTTON_LEFT_Pin:
      Buttons_RecordEdge(BUTTON_ID_LEFT);
      break;
    case BUTTON_RIGHT_Pin:
      Buttons_RecordEdge(BUTTON_ID_RIGHT);
      break;
    default:
      break;
  }
}

#endif /* DRIVERS_BUTTONS_H */