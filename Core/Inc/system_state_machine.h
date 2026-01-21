/**
 ******************************************************************************
 * @file           :  system_state_machine.h
 * @brief          :  System state machine interface for operational control
 *
 * @details        :  Defines the finite state machine that orchestrates system
 *                    operation through configured states and transitions.
 *                    Manages initialization, configuration, adaptation, and
 *                    running states based on user input and system events.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#ifndef CORE_INC_SYSTEM_STATE_MACHINE_H
#define CORE_INC_SYSTEM_STATE_MACHINE_H

#include "system_task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the system state machine
 * @details Sets up state machine with task arguments containing event queues
 *          and shared data access pointers. Initializes state to STATE_INIT.
 * @param args Pointer to SystemTaskArgsTypeDef with queues and context
 * @return void; calls Error_Handler() if args is NULL
 * @note Must be called once during system startup before SystemSM_Run()
 * @see SystemSM_Run, SystemTaskArgsTypeDef, STATE_INIT
 */
void SystemSM_Init(SystemTaskArgsTypeDef *args);

/**
 * @brief Execute one iteration of the state machine
 * @details Processes events from queues, evaluates transition conditions,
 *          executes exit/entry actions for state changes, and updates shared
 *          system context. Should be called periodically by the system task.
 * @return void
 * @note Safe to call multiple times; handles no state change gracefully
 * @see SystemSM_Init, SystemState_t, getNextState
 */
void SystemSM_Run(void);

/**
 * @brief Query current state machine state without mutex
 * @details Internal query of state machine state for logic evaluation.
 *          Does not acquire mutex, for use only within system_state_machine.c
 *          or immediate read without context preservation needed.
 * @return Current SystemState_t value
 * @note Internal use; prefer System_GetState() from system_task.h for public API
 * @see System_GetState, SystemState_t
 */
SystemState_t SystemSM_GetCurrentState(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_SYSTEM_STATE_MACHINE_H */
