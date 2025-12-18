/*
 * File: system_state_machine.c
 *
 * Summary:
 * - System State Machine Implementation
 * - Manages the main operational states of the system.
 */

/* Section: Included Files ****************************************************/
#include "system_state_machine.h"
#include "storage_task.h"
#include "maintenance_task.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "system_task.h"
#include <stdio.h>

/* Section: External Variables ************************************************/
extern osMessageQueueId_t storage2SystemEventQueueHandle;

/* Section: Local Variables ***************************************************/
/* - module variable to save the current state of the System */
static SystemState_t currentSystemState;
/* - module variable to hold the task arguments (queues, context) */
static SystemTaskArgsTypeDef *smArgs = NULL;

/* Section: Local Functions Declarations **************************************/
/* - state handler functions */
static SystemState_t doInitState(void);
static SystemState_t doCodDateTimeState(void);
static SystemState_t doNotInstState(void);
static SystemState_t doAdaptState(void);
static SystemState_t doAdaptFailState(void);
static SystemState_t doRunningState(void);

/* - state machine execution logic */
static SystemState_t getNextState(SystemState_t state);

/* - helper functions */
static void updateSharedState(SystemState_t newState);
static void sendMaintCommand(System2MaintEventTypeDef cmd);

/* Section: Public Functions Definitions **************************************/

void SystemSM_Init(SystemTaskArgsTypeDef *args)
{
    if (args == NULL)
    {
        printf("ERROR: System SM init args NULL\n");
        return;
    }
    smArgs = args;

    /* Set initial state */
    printf("SystemSM: Entering INIT state...\n");
    currentSystemState = STATE_INIT;
    updateSharedState(STATE_INIT);
}

void SystemSM_Run(void)
{
    if (smArgs == NULL)
    {
        return;
    }

    SystemState_t previousSystemState = currentSystemState;
    SystemState_t nextSystemState = getNextState(previousSystemState);

    /* If a state transition occurs, execute exit and entry actions */
    if (nextSystemState != previousSystemState)
    {
        /* Execute "exit" action for the old state */
        switch (previousSystemState)
        {
            case STATE_INIT:
                /* Send init complete event to ViewPresenter on exit from INIT */
                if (smArgs->system2_vp_queue != NULL)
                {
                    System2VPEventTypeDef event = EVT_SYS_INIT_END;
                    osMessageQueuePut(smArgs->system2_vp_queue, &event, 0, 0);
                    printf("SystemSM: Sent EVT_SYS_INIT_END to ViewPresenter on exit from INIT\n");
                }
                break;
            case STATE_COD_DATE_TIME:
            case STATE_NOT_INST:
            case STATE_ADAPT:
            case STATE_ADAPT_FAIL:
            case STATE_RUNNING:
                break;
            default:
                break;
        }

        /* Execute "entry" action for the new state */
        switch (nextSystemState)
        {
            case STATE_INIT:
                printf("SystemSM: Entering INIT state...\n");
                break;
            case STATE_COD_DATE_TIME:
                printf("SystemSM: Entering COD_DATE_TIME state...\n");
                break;
            case STATE_NOT_INST:
                printf("SystemSM: Entering NOT_INST state...\n");
                break;
            case STATE_ADAPT:
                printf("SystemSM: Entering ADAPT state...\n");
                /* Request maint task to perform adaptation */
                sendMaintCommand(EVT_ADAPT_START);
                break;
            case STATE_ADAPT_FAIL:
                printf("SystemSM: Entering ADAPT_FAIL state...\n");
                if (smArgs->system_context_access != NULL && smArgs->system_context_access->mutex != NULL)
                {
                    if (osMutexAcquire(smArgs->system_context_access->mutex, osWaitForever) == osOK)
                    {
                        smArgs->system_context_access->data.adapt_result = ADAPT_RESULT_FAIL;
                        osMutexRelease(smArgs->system_context_access->mutex);
                    }
                }
                break;
            case STATE_RUNNING:
                printf("SystemSM: Entering RUNNING state...\n");
                if (smArgs->system_context_access != NULL && smArgs->system_context_access->mutex != NULL)
                {
                    if (osMutexAcquire(smArgs->system_context_access->mutex, osWaitForever) == osOK)
                    {
                        smArgs->system_context_access->data.adapt_result = ADAPT_RESULT_OK;
                        osMutexRelease(smArgs->system_context_access->mutex);
                    }
                }
                break;
            default:
                break;
        }

        currentSystemState = nextSystemState;
        updateSharedState(nextSystemState);
    }
}

/* Section: Local Functions Definitions ***************************************/

static SystemState_t getNextState(SystemState_t state)
{
    SystemState_t nextState = state;

    switch (state)
    {
        case STATE_INIT:
            nextState = doInitState();
            break;
        case STATE_COD_DATE_TIME:
            nextState = doCodDateTimeState();
            break;
        case STATE_NOT_INST:
            nextState = doNotInstState();
            break;
        case STATE_ADAPT:
            nextState = doAdaptState();
            break;
        case STATE_ADAPT_FAIL:
            nextState = doAdaptFailState();
            break;
        case STATE_RUNNING:
            nextState = doRunningState();
            break;
        default:
            nextState = STATE_INIT; /* Default to safe state */
            break;
    }

    return nextState;
}

static SystemState_t doInitState(void)
{
    SystemState_t nextState = STATE_INIT;
    Storage2SystemEventTypeDef stEvt;

    /* Check Storage Queue */
    if (storage2SystemEventQueueHandle != NULL && 
        osMessageQueueGet(storage2SystemEventQueueHandle, &stEvt, NULL, 0) == osOK)
    {
        if (stEvt == EVT_CFG_LOAD_END)
        {
            nextState = STATE_COD_DATE_TIME;
        }
    }

    /* Also consume VP events to prevent queue full, though ignored in INIT? 
       Original code checked VP queue first. */
    VP2SystemEventTypeDef vpEvt;
    if (smArgs->vp2_system_queue != NULL && 
        osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK)
    {
        /* Ignored in INIT */
    }

    return nextState;
}

static SystemState_t doCodDateTimeState(void)
{
    SystemState_t nextState = STATE_COD_DATE_TIME;
    VP2SystemEventTypeDef vpEvt;

    if (smArgs->vp2_system_queue != NULL && 
        osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK)
    {
        if (vpEvt == EVT_INST_REQ)
        {
            nextState = STATE_NOT_INST;
        }
    }

    return nextState;
}

static SystemState_t doNotInstState(void)
{
    SystemState_t nextState = STATE_NOT_INST;
    VP2SystemEventTypeDef vpEvt;

    if (smArgs->vp2_system_queue != NULL && 
        osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK)
    {
        if (vpEvt == EVT_INST_REQ)
        {
            nextState = STATE_ADAPT;
        }
    }

    return nextState;
}

static SystemState_t doAdaptState(void)
{
    SystemState_t nextState = STATE_ADAPT;
    Maint2SystemEvent_t m2s;

    /* Check Maint Queue */
    if (smArgs->maint2_system_queue != NULL && 
        osMessageQueueGet(smArgs->maint2_system_queue, &m2s, NULL, 0) == osOK)
    {
        if (m2s.result == OK)
        {
            nextState = STATE_RUNNING;
        }
        else if (m2s.result == FAIL)
        {
            nextState = STATE_ADAPT_FAIL;
        }
    }

    /* Consume VP events (ignored in ADAPT?) */
    VP2SystemEventTypeDef vpEvt;
    if (smArgs->vp2_system_queue != NULL && 
        osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK)
    {
        /* Ignored in ADAPT */
    }

    return nextState;
}

static SystemState_t doAdaptFailState(void)
{
    SystemState_t nextState = STATE_ADAPT_FAIL;
    VP2SystemEventTypeDef vpEvt;

    if (smArgs->vp2_system_queue != NULL && 
        osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK)
    {
        if (vpEvt == EVT_ADAPT_RST)
        {
            nextState = STATE_NOT_INST;
        }
    }

    return nextState;
}

static SystemState_t doRunningState(void)
{
    SystemState_t nextState = STATE_RUNNING;
    static uint32_t tick_last = 0;
    
    /* Periodic check */
    uint32_t now = osKernelGetTickCount();
    if ((now - tick_last) > pdMS_TO_TICKS(60000))
    {
        tick_last = now;
        printf("SystemSM: RUNNING periodic tick\n");
    }

    /* Consume VP events (ignored in RUNNING?) */
    VP2SystemEventTypeDef vpEvt;
    if (smArgs->vp2_system_queue != NULL && 
        osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK)
    {
        /* Ignored in RUNNING */
    }

    return nextState;
}

static void updateSharedState(SystemState_t newState)
{
    if (smArgs != NULL && smArgs->system_context_access != NULL && smArgs->system_context_access->mutex != NULL)
    {
        if (osMutexAcquire(smArgs->system_context_access->mutex, 0) == osOK)
        {
            smArgs->system_context_access->data.state = newState;
            osMutexRelease(smArgs->system_context_access->mutex);
        }
    }
}

static void sendMaintCommand(System2MaintEventTypeDef cmd)
{
    if (smArgs != NULL && smArgs->system2_maint_queue != NULL)
    {
        osMessageQueuePut(smArgs->system2_maint_queue, &cmd, 0, 0);
    }
}

SystemState_t SystemSM_GetCurrentState(void)
{
    return currentSystemState;
}
