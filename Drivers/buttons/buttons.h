/**
 ******************************************************************************
 * @file           :  buttons.h
 * @brief          :  Declaration of functions for debounced button input driver
 *                    for MiraTherm radiator thermostat.
 *
 * @details        :  Provides debounced button input handling for multiple
 *                    buttons using GPIO interrupts and system tick-based
 *                    debouncing. Supports press/release event detection with
 *                    timestamp recording. Designed for use with GPIO EXTI
 *                    interrupts to record button edge transitions.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */
#ifndef DRIVERS_BUTTONS_H
#define DRIVERS_BUTTONS_H

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Button identifiers for the three buttons on MiraTherm.
 * 
 * Enumeration of available button inputs:
 * - BUTTON_ID_MIDDLE: Center/confirm button (active low)
 * - BUTTON_ID_LEFT: Left/up button (active high)
 * - BUTTON_ID_RIGHT: Right/down button (active high)
 * - BUTTON_ID_COUNT: Total count of buttons (for iteration)
 */
typedef enum {
  BUTTON_ID_MIDDLE = 0,
  BUTTON_ID_LEFT,
  BUTTON_ID_RIGHT,
  BUTTON_ID_COUNT
} button_id_t;

/**
 * @brief Button action types.
 * 
 * Enumeration of button state transitions:
 * - BUTTON_ACTION_RELEASED: Button released (transitioned to not-pressed)
 * - BUTTON_ACTION_PRESSED: Button pressed (transitioned to pressed)
 */
typedef enum {
  BUTTON_ACTION_RELEASED = 0,
  BUTTON_ACTION_PRESSED
} button_action_t;

/**
 * @brief Button event data structure.
 * 
 * Contains complete information about a detected button event after debouncing.
 * 
 * @note Fields:
 *       - id: Which button triggered the event (button_id_t)
 *       - action: What action occurred (PRESSED or RELEASED)
 *       - timestamp: System tick time when event was confirmed
 */
typedef struct {
  button_id_t id;
  button_action_t action;
  uint32_t timestamp;
} button_event_t;

/**
 * @brief Initialize button driver and read initial states.
 * 
 * Initializes the button driver by:
 * 1. Reading the current GPIO state of all buttons
 * 2. Setting stable state to initial pressed/released condition
 * 3. Clearing pending edge flags
 * 4. Recording startup timestamp for all buttons
 * 
 * This function must be called once during system startup, before
 * enabling GPIO interrupts or polling for events.
 * 
 * @note Not thread-safe. Call only from the initialization context.
 * @see Buttons_Poll()
 * @see Buttons_HandleExtiCallback()
 */
void Buttons_Init(void);

/**
 * @brief Record a button edge transition from GPIO interrupt.
 * 
 * Called by GPIO EXTI interrupt handlers (via Buttons_HandleExtiCallback)
 * when a button GPIO edge is detected. Records the edge timestamp and marks
 * the button as pending debounce confirmation.
 * 
 * The button state will be confirmed during the next Buttons_Poll() call
 * after the debounce delay has elapsed.
 * 
 * @param[in] id Button identifier to record edge for.
 * 
 * @note Safe to call from ISR context (interrupt-safe).
 * @note Must be paired with Buttons_Poll() polling to detect stable state.
 * @see Buttons_Poll()
 * @see Buttons_HandleExtiCallback()
 */
void Buttons_RecordEdge(button_id_t id);

/**
 * @brief Poll for debounced button events.
 * 
 * Checks all pending button edges for debounce timeout and confirms stable
 * state transitions. Returns one debounced event per call if available.
 * 
 * The debounce delay is BUTTONS_DEBOUNCE_MS (typically 50ms). An edge is
 * only reported as a confirmed event if:
 * 1. An edge was recorded (pending flag set)
 * 2. Debounce delay has elapsed since the edge
 * 3. GPIO state has changed from previous stable state
 * 
 * @param[out] event Pointer to button_event_t structure to fill with event data.
 *                   Only modified if function returns true.
 * 
 * @return true if a debounced event was detected and returned in event.
 * @return false if no events are pending or debounce delay has not elapsed.
 * 
 * @note Must be called regularly (typically 10-100ms interval) from main loop.
 * @note Thread-safe: uses interrupt disable for state consistency.
 * @see Buttons_Init()
 * @see Buttons_RecordEdge()
 */
bool Buttons_Poll(button_event_t *event);

/**
 * @brief Get the current stable state of a button.
 * 
 * Returns the stable (debounced) state of the specified button without
 * waiting for or generating events. Useful for checking button state at
 * any time, independent of event polling.
 * 
 * @param[in] id Button identifier to query.
 * 
 * @return true if button is currently in pressed state.
 * @return false if button is in released state or id is invalid.
 * 
 * @note Thread-safe: uses interrupt disable for state consistency.
 * @note The returned state is the stable state, not the instantaneous GPIO level.
 * @see Buttons_Poll()
 */
bool Buttons_GetStableState(button_id_t id);

/**
 * @brief Route GPIO EXTI interrupt to button edge recorder.
 * 
 * Inline helper function to map GPIO pin numbers to button IDs and call
 * Buttons_RecordEdge(). Should be called from GPIO interrupt handlers
 * configured in the system (typically EXTI callbacks).
 * 
 * @param[in] gpio_pin GPIO pin number that generated the interrupt
 *                     (use GPIO_PIN_x constants).
 * 
 * @note Safe to call from ISR context.
 * @note Routes BUTTON_MIDDLE_Pin -> BUTTON_ID_MIDDLE, etc.
 * @see Buttons_RecordEdge()
 */
static inline void Buttons_HandleExtiCallback(uint16_t gpio_pin) {
  switch (gpio_pin) {
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