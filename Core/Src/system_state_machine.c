/**
 ******************************************************************************
 * @file           :  system_state_machine.c
 * @brief          :  Implementation of system state machine logic
 *
 * @details        :  Implements finite state machine handling state transitions,
 *                    event processing, exit/entry actions, and shared context
 *                    updates for operational state control.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#include "system_state_machine.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "main.h"
#include "maintenance_task.h"
#include "storage_task.h"
#include "system_task.h"
#include <stdio.h>

/* External event queue from storage task */
extern osMessageQueueId_t storage2SystemEventQueueHandle;

/* Current system state */
static SystemState_t currentSystemState;

/* Task arguments containing queues and shared data pointers */
static SystemTaskArgsTypeDef *smArgs = NULL;

/* Forward declarations for state handler functions */
static SystemState_t doInitState(void);
static SystemState_t doCodDateTimeState(void);
static SystemState_t doCodScheduleState(void);
static SystemState_t doNotInstState(void);
static SystemState_t doAdaptState(void);
static SystemState_t doAdaptFailState(void);
static SystemState_t doRunningState(void);
static SystemState_t doFactoryRstState(void);

/* State machine logic and helpers */
static SystemState_t getNextState(SystemState_t state);
static void updateSharedState(SystemState_t newState);
static void sendMaintCommand(System2MaintEventTypeDef cmd);

/* Initialize state machine with task arguments */
void SystemSM_Init(SystemTaskArgsTypeDef *args) {
  if (args == NULL) {
    printf("ERROR: System SM init args NULL\n");
    return;
  }
  smArgs = args;

  /* Set initial state */
  printf("SystemSM: Entering INIT state...\n");
  currentSystemState = STATE_INIT;
  updateSharedState(STATE_INIT);
}

/* Execute one iteration of the state machine */
void SystemSM_Run(void) {
  if (smArgs == NULL) {
    return;
  }

  SystemState_t previousSystemState = currentSystemState;
  SystemState_t nextSystemState = getNextState(previousSystemState);

  /* Handle state transitions with exit and entry actions */
  if (nextSystemState != previousSystemState) {
    /* Execute exit action for previous state */
    switch (previousSystemState) {
    case STATE_INIT:
      /* Signal init complete to ViewPresenter on exit */
      if (smArgs->system2_vp_queue != NULL) {
        System2VPEventTypeDef event = EVT_SYS_INIT_END;
        osMessageQueuePut(smArgs->system2_vp_queue, &event, 0, 0);
        printf("SystemSM: Sent EVT_SYS_INIT_END to ViewPresenter on exit from "
               "INIT\n");
      }
      break;
    case STATE_COD_DATE_TIME:
    case STATE_COD_SCHEDULE:
    case STATE_NOT_INST:
    case STATE_ADAPT:
    case STATE_ADAPT_FAIL:
    case STATE_RUNNING:
      break;
    default:
      break;
    }

    /* Execute entry action for new state */
    switch (nextSystemState) {
    case STATE_INIT:
      printf("SystemSM: Entering INIT state...\n");
      break;
    case STATE_COD_DATE_TIME:
      printf("SystemSM: Entering COD_DATE_TIME state...\n");
      break;
    case STATE_COD_SCHEDULE:
      printf("SystemSM: Entering COD_SCHEDULE state...\n");
      break;
    case STATE_NOT_INST:
      printf("SystemSM: Entering NOT_INST state...\n");
      break;
    case STATE_ADAPT:
      printf("SystemSM: Entering ADAPT state...\n");
      /* Request maintenance task to perform adaptation */
      sendMaintCommand(EVT_ADAPT_START);
      break;
    case STATE_ADAPT_FAIL:
      printf("SystemSM: Entering ADAPT_FAIL state...\n");
      if (smArgs->system_context_access != NULL &&
          smArgs->system_context_access->mutex != NULL) {
        if (osMutexAcquire(smArgs->system_context_access->mutex,
                           osWaitForever) == osOK) {
          smArgs->system_context_access->data.adapt_result = ADAPT_RESULT_FAIL;
          osMutexRelease(smArgs->system_context_access->mutex);
        }
      }
      break;
    case STATE_RUNNING:
      printf("SystemSM: Entering RUNNING state...\n");
      if (smArgs->system_context_access != NULL &&
          smArgs->system_context_access->mutex != NULL) {
        if (osMutexAcquire(smArgs->system_context_access->mutex,
                           osWaitForever) == osOK) {
          smArgs->system_context_access->data.adapt_result = ADAPT_RESULT_OK;
          osMutexRelease(smArgs->system_context_access->mutex);
        }
      }
      break;
    case STATE_FACTORY_RST:
      printf("SystemSM: Entering FACTORY_RST state...\n");
      if (smArgs->system2_storage_queue != NULL) {
        System2StorageEventTypeDef evt = EVT_CFG_RST_REQ;
        osMessageQueuePut(smArgs->system2_storage_queue, &evt, 0, 0);
      }
      break;
    default:
      break;
    }

    currentSystemState = nextSystemState;
    updateSharedState(nextSystemState);
  }
}

/* Determine next state based on current state and queued events */
static SystemState_t getNextState(SystemState_t state) {
  SystemState_t nextState = state;

  switch (state) {
  case STATE_INIT:
    nextState = doInitState();
    break;
  case STATE_COD_DATE_TIME:
    nextState = doCodDateTimeState();
    break;
  case STATE_COD_SCHEDULE:
    nextState = doCodScheduleState();
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
  case STATE_FACTORY_RST:
    nextState = doFactoryRstState();
    break;
  default:
    nextState = STATE_INIT; /* Default to safe state */
    break;
  }

  return nextState;
}

/* INIT state: wait for config load from storage */
static SystemState_t doInitState(void) {
  SystemState_t nextState = STATE_INIT;
  Storage2SystemEventTypeDef stEvt;

  /* Check for storage config load completion */
  if (storage2SystemEventQueueHandle != NULL &&
      osMessageQueueGet(storage2SystemEventQueueHandle, &stEvt, NULL, 0) ==
          osOK) {
    if (stEvt == EVT_CFG_LOAD_END) {
      nextState = STATE_COD_DATE_TIME;
    }
  }

  /* Consume VP events to prevent queue overflow */
  VP2SystemEventTypeDef vpEvt;
  if (smArgs->vp2_system_queue != NULL &&
      osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK) {
    /* Ignored in INIT */
  }

  return nextState;
}

/* COD_DATE_TIME state: wait for date/time configuration completion */
static SystemState_t doCodDateTimeState(void) {
  SystemState_t nextState = STATE_COD_DATE_TIME;
  VP2SystemEventTypeDef vpEvt;

  if (smArgs->vp2_system_queue != NULL &&
      osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK) {
    if (vpEvt == EVT_COD_DT_END) {
      nextState = STATE_COD_SCHEDULE;
    }
  }

  return nextState;
}

/* COD_SCHEDULE state: wait for schedule configuration completion */
static SystemState_t doCodScheduleState(void) {
  SystemState_t nextState = STATE_COD_SCHEDULE;
  VP2SystemEventTypeDef vpEvt;

  if (smArgs->vp2_system_queue != NULL &&
      osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK) {
    if (vpEvt == EVT_COD_SH_END) {
      nextState = STATE_NOT_INST;
    }
  }

  return nextState;
}

/* NOT_INST state: wait for user to request adaptation */
static SystemState_t doNotInstState(void) {
  SystemState_t nextState = STATE_NOT_INST;
  VP2SystemEventTypeDef vpEvt;

  if (smArgs->vp2_system_queue != NULL &&
      osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK) {
    if (vpEvt == EVT_INST_REQ) {
      nextState = STATE_ADAPT;
    }
  }

  return nextState;
}

/* ADAPT state: wait for adaptation result from maintenance task */
static SystemState_t doAdaptState(void) {
  SystemState_t nextState = STATE_ADAPT;
  Maint2SystemEvent_t m2s;

  /* Check maintenance result */
  if (smArgs->maint2_system_queue != NULL &&
      osMessageQueueGet(smArgs->maint2_system_queue, &m2s, NULL, 0) == osOK) {
    if (m2s.result == OK) {
      nextState = STATE_RUNNING;
    } else if (m2s.result == FAIL) {
      nextState = STATE_ADAPT_FAIL;
    }
  }

  /* Consume VP events to prevent queue overflow */
  VP2SystemEventTypeDef vpEvt;
  if (smArgs->vp2_system_queue != NULL &&
      osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK) {
    /* Ignored in ADAPT */
  }

  return nextState;
}

/* ADAPT_FAIL state: wait for user to retry or abandon adaptation */
static SystemState_t doAdaptFailState(void) {
  SystemState_t nextState = STATE_ADAPT_FAIL;
  VP2SystemEventTypeDef vpEvt;

  if (smArgs->vp2_system_queue != NULL &&
      osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK) {
    if (vpEvt == EVT_ADAPT_RST_REQ) {
      nextState = STATE_NOT_INST;
    }
  }

  return nextState;
}

/* RUNNING state: manage active heating control and schedule transitions */
static SystemState_t doRunningState(void) {
  SystemState_t nextState = STATE_RUNNING;
  static uint8_t last_slot_end_hour =
      0xFF; /* Track previous slot to detect transitions */
  static uint8_t last_slot_end_minute = 0xFF;

  /* Check for boost mode timeout (300 seconds) */
  if (smArgs && smArgs->system_context_access) {
    if (osMutexAcquire(smArgs->system_context_access->mutex, 10) == osOK) {
      if (smArgs->system_context_access->data.mode == MODE_BOOST) {
        uint32_t elapsed_ticks =
            osKernelGetTickCount() -
            smArgs->system_context_access->data.boost_begin_time;
        if (elapsed_ticks >= pdMS_TO_TICKS(300000)) /* 300 seconds */
        {
          /* Restore mode before boost */
          SystemMode_t previous_mode =
              smArgs->system_context_access->data.mode_before_boost;
          smArgs->system_context_access->data.mode = previous_mode;
          printf(
              "SystemSM: Boost mode timeout - restoring previous mode (%d)\n",
              previous_mode);
        }
      }
      osMutexRelease(smArgs->system_context_access->mutex);
    }
  }

  /* Calculate target temperature and schedule slot end time */
  if (smArgs && smArgs->config_access && smArgs->system_context_access) {
    extern RTC_HandleTypeDef hrtc;
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    float target_temp = 20.0f;
    uint8_t end_h = 0, end_m = 0;
    SystemMode_t current_mode = MODE_AUTO;

    /* Read current mode */
    if (osMutexAcquire(smArgs->system_context_access->mutex, 10) == osOK) {
      current_mode = smArgs->system_context_access->data.mode;
      osMutexRelease(smArgs->system_context_access->mutex);
    }

    /* Set target temperature based on current operating mode */
    if (current_mode == MODE_AUTO) {
      /* AUTO mode: calculate from active schedule slot */
      if (osMutexAcquire(smArgs->config_access->mutex, 10) == osOK) {
        ConfigTypeDef *cfg = &smArgs->config_access->data;
        bool found = false;

        int current_mins = sTime.Hours * 60 + sTime.Minutes;

        for (int i = 0; i < cfg->DailySchedule.NumTimeSlots; i++) {
          TimeSlotTypeDef *slot = &cfg->DailySchedule.TimeSlots[i];

          int start_mins = slot->StartHour * 60 + slot->StartMinute;
          int end_mins = slot->EndHour * 60 + slot->EndMinute;

          if (current_mins >= start_mins && current_mins < end_mins) {
            target_temp = slot->Temperature;
            end_h = slot->EndHour;
            end_m = slot->EndMinute;
            found = true;
            break;
          }
        }

        if (!found) {
          target_temp = 20.0f;
          end_h = 0;
          end_m = 0;
        }

        osMutexRelease(smArgs->config_access->mutex);
      }
    } else if (current_mode == MODE_MANUAL) {
      /* MANUAL mode: use fixed manual temperature */
      if (osMutexAcquire(smArgs->config_access->mutex, 10) == osOK) {
        target_temp = smArgs->config_access->data.ManualTargetTemp;
        osMutexRelease(smArgs->config_access->mutex);
      }
      end_h = 0xFF; /* No slot tracking in manual */
      end_m = 0xFF;
    } else if (current_mode == MODE_BOOST) {
      /* BOOST mode: maximum heating (30°C) */
      target_temp = 30.0f;
      end_h = 0xFF; /* No slot tracking in boost */
      end_m = 0xFF;
    }

    /* Update shared context with calculated values */
    if (osMutexAcquire(smArgs->system_context_access->mutex, 10) == osOK) {
      /* Only clear temporary override in AUTO mode on slot change */
      if (current_mode == MODE_AUTO) {
        bool slot_changed =
            (end_h != last_slot_end_hour) || (end_m != last_slot_end_minute);
        if (slot_changed &&
            smArgs->system_context_access->data.temporary_target_temp != 0) {
          smArgs->system_context_access->data.temporary_target_temp = 0;
          printf("SystemSM: Cleared temporary target temperature (slot changed "
                 "from %02u:%02u to %02u:%02u)\n",
                 last_slot_end_hour, last_slot_end_minute, end_h, end_m);
        }

        smArgs->system_context_access->data.target_temp = target_temp;
        smArgs->system_context_access->data.slot_end_hour = end_h;
        smArgs->system_context_access->data.slot_end_minute = end_m;
      } else if (current_mode == MODE_MANUAL || current_mode == MODE_BOOST) {
        /* MANUAL/BOOST: only update target, don't modify slot info */
        smArgs->system_context_access->data.target_temp = target_temp;
      }

      osMutexRelease(smArgs->system_context_access->mutex);

      /* Update slot tracking (AUTO mode only) */
      if (current_mode == MODE_AUTO) {
        last_slot_end_hour = end_h;
        last_slot_end_minute = end_m;
      }
    }
  }

  /* Check for factory reset request */
  VP2SystemEventTypeDef vpEvt;
  if (smArgs->vp2_system_queue != NULL &&
      osMessageQueueGet(smArgs->vp2_system_queue, &vpEvt, NULL, 0) == osOK) {
    if (vpEvt == EVT_FACTORY_RST_REQ) {
      nextState = STATE_FACTORY_RST;
    }
  }

  return nextState;
}

/* FACTORY_RST state: wait for storage task to complete reset */
static SystemState_t doFactoryRstState(void) {
  SystemState_t nextState = STATE_FACTORY_RST;
  Storage2SystemEventTypeDef stEvt;

  /* Check for factory reset completion */
  if (storage2SystemEventQueueHandle != NULL &&
      osMessageQueueGet(storage2SystemEventQueueHandle, &stEvt, NULL, 0) ==
          osOK) {
    if (stEvt == EVT_CFG_RST_END) {
      printf("SystemSM: Factory Reset Complete. Resetting MCU...\n");

      /* Reset backup domain (RTC) */
      HAL_PWR_EnableBkUpAccess();
      __HAL_RCC_BACKUPRESET_FORCE();
      __HAL_RCC_BACKUPRESET_RELEASE();

      /* Reset MCU */
      NVIC_SystemReset();
    }
  }
  return nextState;
}

/* Update shared system context with new state */
static void updateSharedState(SystemState_t newState) {
  if (smArgs != NULL && smArgs->system_context_access != NULL &&
      smArgs->system_context_access->mutex != NULL) {
    if (osMutexAcquire(smArgs->system_context_access->mutex, 0) == osOK) {
      smArgs->system_context_access->data.state = newState;
      osMutexRelease(smArgs->system_context_access->mutex);
    }
  }
}

/* Send command to maintenance task via queue */
static void sendMaintCommand(System2MaintEventTypeDef cmd) {
  if (smArgs != NULL && smArgs->system2_maint_queue != NULL) {
    osMessageQueuePut(smArgs->system2_maint_queue, &cmd, 0, 0);
  }
}

/* Query current state without mutex (internal use) */
SystemState_t SystemSM_GetCurrentState(void) { return currentSystemState; }
