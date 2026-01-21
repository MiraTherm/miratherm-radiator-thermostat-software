/**
 ******************************************************************************
 * @file           :  motor.c
 * @brief          :  Implementation of motor control interface for DRV8833
 *                    or similar H-bridge motor driver.
 *
 * @details        :  Provides control pin manipulation and state management
 *                    for motor operation. Implements coast, forward, backward,
 *                    and brake states through GPIO control.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#include "motor.h"

/* Current motor control state cache */
static MotorStateTypeDef s_current_state = MOTOR_COAST;

/* Apply motor control pin states to DRV8833 or similar driver */
static void motor_apply_pins(GPIO_PinState a_state, GPIO_PinState b_state) {
  HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, a_state);
  HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, b_state);
}

/* Initialize the motor driver and reset to coast state */
void Motor_Init(void) {
  motor_apply_pins(GPIO_PIN_RESET, GPIO_PIN_RESET);
  s_current_state = MOTOR_COAST;
}

/* Set the motor to a desired control state */
void Motor_SetState(MotorStateTypeDef state) {
  switch (state) {
  case MOTOR_COAST:
    /* Both inputs low -> coast/fast decay (Table 1). */
    motor_apply_pins(GPIO_PIN_RESET, GPIO_PIN_RESET);
    break;
  case MOTOR_FORWARD:
    motor_apply_pins(GPIO_PIN_SET, GPIO_PIN_RESET);
    break;
  case MOTOR_BACKWARD:
    motor_apply_pins(GPIO_PIN_RESET, GPIO_PIN_SET);
    break;
  case MOTOR_BRAKE:
    /* Both inputs high -> brake/slow decay. */
    motor_apply_pins(GPIO_PIN_SET, GPIO_PIN_SET);
    break;
  default:
    motor_apply_pins(GPIO_PIN_RESET, GPIO_PIN_RESET);
    break;
  }

  s_current_state =
      (state < MOTOR_COAST || state > MOTOR_BRAKE) ? MOTOR_COAST : state;
}

/* Get the current motor control state */
MotorStateTypeDef Motor_GetState(void) { return s_current_state; }
