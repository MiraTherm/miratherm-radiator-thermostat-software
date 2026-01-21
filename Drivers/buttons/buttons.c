/**
 ******************************************************************************
 * @file           :  buttons.c
 * @brief          :  Implementation of debounced button input driver.
 *
 * @details        :  Provides debounce logic using GPIO EXTI interrupts and
 *                    system tick timestamps. Button presses/releases are
 *                    confirmed after BUTTONS_DEBOUNCE_MS delay. Three buttons
 *                    supported: middle (active low), left (active high), right
 *                    (active high).
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */
#include "buttons.h"

/* Debounce delay in milliseconds */
#define BUTTONS_DEBOUNCE_MS 50U

/* Button pin mapping: GPIO port, pin, and active level */
typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
  GPIO_PinState pressed_level;
} button_pin_map_t;

/* Button state tracking: pending edge, stable state, and timestamp */
typedef struct {
  volatile bool pending;
  volatile bool stable_state;
  volatile uint32_t last_edge_tick;
} button_state_t;

/* Static pin configuration for all buttons */
static const button_pin_map_t s_button_pins[BUTTON_ID_COUNT] = {
    [BUTTON_ID_MIDDLE] = {BUTTON_MIDDLE_GPIO_Port, BUTTON_MIDDLE_Pin,
                          GPIO_PIN_RESET},
    [BUTTON_ID_LEFT] = {BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin, GPIO_PIN_SET},
    [BUTTON_ID_RIGHT] = {BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin,
                         GPIO_PIN_SET},
};

/* Button state array: one entry per button */
static button_state_t s_button_states[BUTTON_ID_COUNT];

/* Read GPIO pin and return true if button is pressed (matches pressed_level) */
static inline bool button_read_pressed(const button_pin_map_t *mapping) {
  return HAL_GPIO_ReadPin(mapping->port, mapping->pin) ==
         mapping->pressed_level;
}

/* Initialize button states to current GPIO levels */
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

/* Record button edge timestamp and mark as pending debounce */
void Buttons_RecordEdge(button_id_t id) {
  if (id >= BUTTON_ID_COUNT) {
    return;
  }

  button_state_t *state = &s_button_states[id];
  state->pending = true;
  state->last_edge_tick = HAL_GetTick();
}

/* Poll all pending button edges, apply debounce, and return confirmed events */
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

    /* Disable interrupts briefly to check for race condition */
    __disable_irq();
    if (state->last_edge_tick != edge_tick) {
      __enable_irq();
      continue;
    }

    const bool previous_pressed = state->stable_state;
    state->pending = false;

    if (current_pressed != previous_pressed) {
      state->stable_state = current_pressed;
    }
    __enable_irq();

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

/* Get current stable (debounced) state of a button without generating events */
bool Buttons_GetStableState(button_id_t id) {
  if (id >= BUTTON_ID_COUNT) {
    return false;
  }

  /* Single volatile read is atomic on ARM Cortex-M, no critical section needed */
  return s_button_states[id].stable_state;
}