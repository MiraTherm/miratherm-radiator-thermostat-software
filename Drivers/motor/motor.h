/**
 ******************************************************************************
 * @file           :  motor.h
 * @brief          :  Declaration of functions to interface with the DRV8833
 *                    or similar motor driver for MiraTherm radiator thermostat.
 *
 * @details        :  This module provides control interface for the DRV8833
 *                    or similar H-bridge motor driver. Supports coast, forward,
 *                    backward, and brake motor states.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */
#ifndef DRIVERS_MOTOR_MOTOR_H
#define DRIVERS_MOTOR_MOTOR_H

#include "main.h"

/**
 * @brief Motor control states for DRV8833 or similar driver.
 * 
 * Defines the four possible motor control modes supported by the DRV8833
 * or similar H-bridge motor driver.
 */
typedef enum {
  /** @brief Coast mode (fast decay). */
  MOTOR_COAST = 0,
  
  /** @brief Forward mode. */
  MOTOR_FORWARD,
  
  /** @brief Backward mode. */
  MOTOR_BACKWARD,
  
  /** @brief Brake mode (slow decay). */
  MOTOR_BRAKE
} MotorStateTypeDef;

/**
 * @brief Initialize the motor driver and reset to coast state.
 * 
 * Sets the motor to its default state (coast).
 * This function must be called once during system startup before any
 * motor operation.
 * 
 * @note Not thread-safe. Call only from the initialization context.
 * @see Motor_SetState()
 */
void Motor_Init(void);

/**
 * @brief Set the motor to a desired control state.
 * 
 * Applies the specified motor state by writing the corresponding values
 * to the motor driver.
 * 
 * @param[in] state Desired motor state (MOTOR_COAST, MOTOR_FORWARD,
 *                  MOTOR_BACKWARD, or MOTOR_BRAKE).
 * 
 * @note Thread-safe if GPIO operations are atomic (typical for STM32).
 * @see Motor_GetState()
 * @see MotorStateTypeDef
 */
void Motor_SetState(MotorStateTypeDef state);

/**
 * @brief Get the current motor control state.
 * 
 * Returns the last motor state that was successfully set via Motor_SetState().
 * This reflects the software state, not necessarily the actual motor motion
 * (which may be affected by load, inertia, etc.).
 * 
 * @return Current motor state from MotorStateTypeDef enum.
 * 
 * @note This returns the cached software state, not a GPIO sensor reading.
 * @see Motor_SetState()
 */
MotorStateTypeDef Motor_GetState(void);

#endif /* DRIVERS_MOTOR_MOTOR_H */
