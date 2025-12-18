/*
 * File: system_state_machine.h
 *
 * Summary:
 * - System State Machine
 * - Manages the main operational states of the system.
 */

#ifndef CORE_INC_SYSTEM_STATE_MACHINE_H
#define CORE_INC_SYSTEM_STATE_MACHINE_H

/* Section: Included Files ****************************************************/
#include "system_task.h"
#include <stdbool.h>

/* Section: Public Functions Declarations *************************************/

/**
 * @brief Initialize the system state machine.
 * @param args Pointer to the system task arguments containing queues and context.
 */
void SystemSM_Init(SystemTaskArgsTypeDef *args);

/**
 * @brief Execute one iteration of the state machine.
 *        Should be called periodically.
 */
void SystemSM_Run(void);

/**
 * @brief Get the current state of the state machine.
 */
SystemState_t SystemSM_GetCurrentState(void);

#endif /* CORE_INC_SYSTEM_STATE_MACHINE_H */
