#include "buttons.h"

#define BUTTONS_DEBOUNCE_MS 50U

typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
  GPIO_PinState pressed_level;
} button_pin_map_t;

typedef struct {
  volatile bool pending;
  volatile bool stable_state;
  volatile uint32_t last_edge_tick;
} button_state_t;

static const button_pin_map_t s_button_pins[BUTTON_ID_COUNT] = {
    [BUTTON_ID_MIDDLE] = {BUTTON_MIDDLE_GPIO_Port, BUTTON_MIDDLE_Pin,
                          GPIO_PIN_RESET},
    [BUTTON_ID_LEFT] = {BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin, GPIO_PIN_SET},
    [BUTTON_ID_RIGHT] = {BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin,
                         GPIO_PIN_SET},
};

static button_state_t s_button_states[BUTTON_ID_COUNT];

static inline bool button_read_pressed(const button_pin_map_t *mapping) {
  return HAL_GPIO_ReadPin(mapping->port, mapping->pin) ==
         mapping->pressed_level;
}

void Buttons_Init(void) {
  const uint32_t start_tick = HAL_GetTick();

  for (button_id_t id = BUTTON_ID_MIDDLE; id < BUTTON_ID_COUNT; ++id) {
    const button_pin_map_t *mapping = &s_button_pins[id];
    const bool pressed = button_read_pressed(mapping);

    s_button_states[id].stable_state = pressed;
    s_button_states[id].pending = false;
    s_button_states[id].last_edge_tick = start_tick;
  }
}

void Buttons_RecordEdge(button_id_t id) {
  if (id >= BUTTON_ID_COUNT) {
    return;
  }

  button_state_t *state = &s_button_states[id];
  state->pending = true;
  state->last_edge_tick = HAL_GetTick();
}

static uint32_t button_primask_save(void) { return __get_PRIMASK(); }

static void button_primask_restore(uint32_t mask) { __set_PRIMASK(mask); }

bool Buttons_Poll(button_event_t *event) {
  if (event == NULL) {
    return false;
  }

  const uint32_t now = HAL_GetTick();

  for (button_id_t id = BUTTON_ID_MIDDLE; id < BUTTON_ID_COUNT; ++id) {
    button_state_t *state = &s_button_states[id];

    if (!state->pending) {
      continue;
    }

    const uint32_t edge_tick = state->last_edge_tick;
    if ((now - edge_tick) < BUTTONS_DEBOUNCE_MS) {
      continue;
    }

    const button_pin_map_t *mapping = &s_button_pins[id];
    const bool current_pressed = button_read_pressed(mapping);

    const uint32_t saved_mask = button_primask_save();
    __disable_irq();

    if (state->last_edge_tick != edge_tick) {
      button_primask_restore(saved_mask);
      continue;
    }

    const bool previous_pressed = state->stable_state;
    state->pending = false;

    if (current_pressed != previous_pressed) {
      state->stable_state = current_pressed;
    }

    button_primask_restore(saved_mask);

    if (current_pressed != previous_pressed) {
      event->id = id;
      event->action =
          current_pressed ? BUTTON_ACTION_PRESSED : BUTTON_ACTION_RELEASED;
      event->timestamp = now;
      return true;
    }
  }

  return false;
}

bool Buttons_GetStableState(button_id_t id) {
  if (id >= BUTTON_ID_COUNT) {
    return false;
  }

  const uint32_t saved_mask = button_primask_save();
  __disable_irq();

  const bool state = s_button_states[id].stable_state;

  button_primask_restore(saved_mask);
  return state;
}