/**
 ******************************************************************************
 * @file           :  view_presenter_router.c
 * @brief          :  Implementation of MVP router for UI navigation
 *
 * @details        :  Manages view/presenter lifecycle, state-driven routing,
 *                    input event dispatch, and periodic UI updates.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#include "view_presenter_router.h"
#include "boost_presenter.h"
#include "boost_view.h"
#include "change_schedule_presenter.h"
#include "change_schedule_view.h"
#include "cmsis_os2.h"
#include "factory_reset_presenter.h"
#include "home_presenter.h"
#include "home_view.h"
#include "loading_presenter.h"
#include "loading_view.h"
#include "menu_presenter.h"
#include "menu_view.h"
#include "set_date_time_presenter.h"
#include "set_date_time_view.h"
#include "set_temp_offset_presenter.h"
#include "set_value_view.h"
#include "task_debug.h"
#include "waiting_presenter.h"
#include "waiting_view.h"
#if VIEW_PRESENTER_TASK_DEBUG_LEDS
#include "stm32wbxx_nucleo.h"
#endif
#include <stddef.h>
#include <stdint.h>

/* Global router state with all view/presenter pairs and queues */
typedef struct {
  RouteTypeDef current_route;

  /* Configuration screens */
  SetDateTimePresenter_t *dt_presenter;
  SetDateTimeView_t *dt_view;

  ChangeSchedulePresenter_t *sch_presenter;
  ChangeScheduleView_t *sch_view;

  /* Initialization and progress screens */
  LoadingPresenter_t *loading_presenter;
  LoadingView_t *loading_view;

  WaitingPresenter_t *waiting_presenter;
  WaitingView_t *waiting_view;

  LoadingPresenter_t *adapt_presenter;
  LoadingView_t *adapt_view;

  WaitingPresenter_t *adapt_fail_presenter;
  WaitingView_t *adapt_fail_view;

  /* Operating screens */
  HomePresenter_t *home_presenter;
  HomeView_t *home_view;

  BoostPresenter_t *boost_presenter;
  BoostView_t *boost_view;

  MenuPresenter_t *menu_presenter;
  MenuView_t *menu_view;

  /* Settings screens */
  SetTempOffsetPresenter_t *temp_offset_presenter;
  SetValueView_t *temp_offset_view;

  FactoryResetPresenter_t *factory_reset_presenter;

  /* System communication and shared data */
  osMessageQueueId_t vp2system_queue;
  SystemContextAccessTypeDef *system_context;
  ConfigAccessTypeDef *config_access;
  SensorValuesAccessTypeDef *sensor_values_access;
} Router_State_t;

/* Global router state instance */
static Router_State_t g_router_state = {
    .current_route = ROUTE_INIT,
    .dt_presenter = NULL,
    .dt_view = NULL,
    .sch_presenter = NULL,
    .sch_view = NULL,
    .loading_presenter = NULL,
    .loading_view = NULL,
    .waiting_presenter = NULL,
    .waiting_view = NULL,
    .adapt_presenter = NULL,
    .adapt_view = NULL,
    .adapt_fail_presenter = NULL,
    .adapt_fail_view = NULL,
    .home_presenter = NULL,
    .home_view = NULL,
    .boost_presenter = NULL,
    .boost_view = NULL,
    .menu_presenter = NULL,
    .menu_view = NULL,
    .temp_offset_presenter = NULL,
    .temp_offset_view = NULL,
    .factory_reset_presenter = NULL,
    .vp2system_queue = NULL,
    .system_context = NULL,
    .config_access = NULL,
    .sensor_values_access = NULL};

/* Update debug LEDs based on button input (debug feature) */
static void Router_UpdateDebugLeds(const Input2VPEvent_t *event) {
#if VIEW_PRESENTER_TASK_DEBUG_LEDS
  if (!event)
    return;

  uint16_t target_led = 0;
  bool should_update = true;
  switch (event->type) {
  case EVT_LEFT_BTN:
    target_led = LED3;
    break;
  case EVT_MIDDLE_BTN:
    target_led = LED2;
    break;
  case EVT_RIGHT_BTN:
    target_led = LED1;
    break;
  default:
    should_update = false;
    break;
  }

  if (!should_update)
    return;

  if (event->button_action == BUTTON_ACTION_PRESSED)
    BSP_LED_On(target_led);
  else if (event->button_action == BUTTON_ACTION_RELEASED)
    BSP_LED_Off(target_led);
#else
  (void)event;
#endif
}

/* Query current system state with mutex protection */
static SystemState_t Router_GetSystemState(void) {
  SystemState_t state = STATE_INIT;
  if (g_router_state.system_context && g_router_state.system_context->mutex) {
    if (osMutexAcquire(g_router_state.system_context->mutex, osWaitForever) ==
        osOK) {
      state = g_router_state.system_context->data.state;
      osMutexRelease(g_router_state.system_context->mutex);
    } else {
      printf("Router: Failed to acquire system context mutex\n");
    }
  } else {
    printf("Router: system_context or mutex is NULL\n");
  }
  return state;
}

/* Send user action event to system task */
static void Router_SendSystemEvent(VP2SystemEventTypeDef event) {
  if (g_router_state.vp2system_queue) {
    osMessageQueuePut(g_router_state.vp2system_queue, &event, 0, 0);
  }
}

/**
 * @brief Initialize the router and activate initial route
 */
void Router_Init(osMessageQueueId_t vp2system_queue,
                 SystemContextAccessTypeDef *system_context,
                 ConfigAccessTypeDef *config_access,
                 SensorValuesAccessTypeDef *sensor_values_access) {
  g_router_state.vp2system_queue = vp2system_queue;
  g_router_state.system_context = system_context;
  g_router_state.config_access = config_access;
  g_router_state.sensor_values_access = sensor_values_access;

  /* Start in INIT route */
  g_router_state.current_route = ROUTE_INIT;

  /* Initialize loading view for INIT */
  g_router_state.loading_view =
      LoadingView_Init("Initialize", LV_ALIGN_CENTER, 0);
  if (g_router_state.loading_view) {
    g_router_state.loading_presenter =
        LoadingPresenter_Init(g_router_state.loading_view);
    if (g_router_state.loading_presenter) {
      LoadingPresenter_Run(g_router_state.loading_presenter,
                           osKernelGetTickCount());
    }
  }
}

/**
 * @brief Deinitialize the router
 */
void Router_Deinit(void) {
  if (g_router_state.dt_view) {
    SetDateTimeView_Deinit(g_router_state.dt_view);
    g_router_state.dt_view = NULL;
  }

  if (g_router_state.dt_presenter) {
    SetDateTimePresenter_Deinit(g_router_state.dt_presenter);
    g_router_state.dt_presenter = NULL;
  }

  if (g_router_state.loading_view) {
    LoadingView_Deinit(g_router_state.loading_view);
    g_router_state.loading_view = NULL;
  }

  if (g_router_state.loading_presenter) {
    LoadingPresenter_Deinit(g_router_state.loading_presenter);
    g_router_state.loading_presenter = NULL;
  }

  if (g_router_state.waiting_view) {
    WaitingView_Deinit(g_router_state.waiting_view);
    g_router_state.waiting_view = NULL;
  }

  if (g_router_state.waiting_presenter) {
    WaitingPresenter_Deinit(g_router_state.waiting_presenter);
    g_router_state.waiting_presenter = NULL;
  }

  if (g_router_state.adapt_view) {
    LoadingView_Deinit(g_router_state.adapt_view);
    g_router_state.adapt_view = NULL;
  }

  if (g_router_state.adapt_presenter) {
    LoadingPresenter_Deinit(g_router_state.adapt_presenter);
    g_router_state.adapt_presenter = NULL;
  }

  if (g_router_state.adapt_fail_view) {
    WaitingView_Deinit(g_router_state.adapt_fail_view);
    g_router_state.adapt_fail_view = NULL;
  }

  if (g_router_state.adapt_fail_presenter) {
    WaitingPresenter_Deinit(g_router_state.adapt_fail_presenter);
    g_router_state.adapt_fail_presenter = NULL;
  }

  if (g_router_state.home_view) {
    HomeView_Deinit(g_router_state.home_view);
    g_router_state.home_view = NULL;
  }

  if (g_router_state.home_presenter) {
    HomePresenter_Deinit(g_router_state.home_presenter);
    g_router_state.home_presenter = NULL;
  }

  if (g_router_state.menu_view) {
    MenuView_Deinit(g_router_state.menu_view);
    g_router_state.menu_view = NULL;
  }

  if (g_router_state.menu_presenter) {
    MenuPresenter_Deinit(g_router_state.menu_presenter);
    g_router_state.menu_presenter = NULL;
  }

  if (g_router_state.temp_offset_view) {
    SetValueView_Deinit(g_router_state.temp_offset_view);
    g_router_state.temp_offset_view = NULL;
  }

  if (g_router_state.temp_offset_presenter) {
    SetTempOffsetPresenter_Deinit(g_router_state.temp_offset_presenter);
    g_router_state.temp_offset_presenter = NULL;
  }

  if (g_router_state.factory_reset_presenter) {
    FactoryResetPresenter_Deinit(g_router_state.factory_reset_presenter);
    g_router_state.factory_reset_presenter = NULL;
  }
}

/**
 * @brief Handle input event in current route
 */
void Router_HandleEvent(const Input2VPEvent_t *event) {
  if (!event)
    return;

  Router_UpdateDebugLeds(event);

  if (g_router_state.current_route == ROUTE_DATE_TIME) {
    if (g_router_state.dt_presenter) {
      /* Presenter handles the event and updates itself, which triggers view
       * updates */
      SetDateTimePresenter_HandleEvent(g_router_state.dt_presenter, event);

      /* Check if date/time setup is complete */
      if (SetDateTimePresenter_IsComplete(g_router_state.dt_presenter)) {
        /* Transition to schedule configuration within same state */
        Router_GoToRoute(ROUTE_CHANGE_SCHEDULE);
        return;
      }
    }
  } else if (g_router_state.current_route == ROUTE_CHANGE_SCHEDULE) {
    if (g_router_state.sch_presenter) {
      ChangeSchedulePresenter_HandleEvent(g_router_state.sch_presenter, event);

      if (ChangeSchedulePresenter_IsCancelled(g_router_state.sch_presenter)) {
        SystemState_t state = Router_GetSystemState();
        if (state == STATE_RUNNING) {
          Router_GoToRoute(ROUTE_MENU);
        } else {
          /* In setup mode, go back to date/time configuration */
          Router_GoToRoute(ROUTE_DATE_TIME);
        }
        return;
      }

      if (ChangeSchedulePresenter_IsComplete(g_router_state.sch_presenter)) {
        SystemState_t state = Router_GetSystemState();
        if (state == STATE_RUNNING) {
          /* In running mode, go back to Menu */
          Router_GoToRoute(ROUTE_MENU);
        } else {
          /* In setup mode, signal System that both COD steps are done */
          Router_SendSystemEvent(EVT_COD_END);
        }
        return;
      }
    }
  } else if (g_router_state.current_route == ROUTE_NOT_INST) {
    if (g_router_state.waiting_presenter) {
      WaitingPresenter_HandleEvent(g_router_state.waiting_presenter, event);
      if (WaitingPresenter_IsComplete(g_router_state.waiting_presenter)) {
        Router_SendSystemEvent(EVT_INST_REQ); /* Moves to STATE_ADAPT */
      }
    }
  } else if (g_router_state.current_route == ROUTE_ADAPT_FAIL) {
    if (g_router_state.adapt_fail_presenter) {
      WaitingPresenter_HandleEvent(g_router_state.adapt_fail_presenter, event);
      if (WaitingPresenter_IsComplete(g_router_state.adapt_fail_presenter)) {
        Router_SendSystemEvent(EVT_ADAPT_RST_REQ); /* Moves to STATE_NOT_INST */
      }
    }
  } else if (g_router_state.current_route == ROUTE_HOME) {
    if (g_router_state.home_presenter) {
      HomePresenter_HandleEvent(g_router_state.home_presenter, event);
    }
  } else if (g_router_state.current_route == ROUTE_BOOST) {
    if (g_router_state.boost_presenter) {
      BoostPresenter_HandleEvent(g_router_state.boost_presenter, event);
    }
  } else if (g_router_state.current_route == ROUTE_MENU) {
    if (g_router_state.menu_presenter) {
      MenuPresenter_HandleEvent(g_router_state.menu_presenter, event);
    }
  } else if (g_router_state.current_route == ROUTE_EDIT_TEMP_OFFSET) {
    if (g_router_state.temp_offset_presenter) {
      SetTempOffsetPresenter_HandleEvent(g_router_state.temp_offset_presenter,
                                         event);

      if (SetTempOffsetPresenter_IsCancelled(
              g_router_state.temp_offset_presenter)) {
        Router_GoToRoute(ROUTE_MENU);
        return;
      }

      if (SetTempOffsetPresenter_IsComplete(
              g_router_state.temp_offset_presenter)) {
        Router_GoToRoute(ROUTE_MENU);
      }
    }
  } else if (g_router_state.current_route == ROUTE_FACTORY_RESET) {
    if (g_router_state.factory_reset_presenter) {
      FactoryResetPresenter_HandleEvent(g_router_state.factory_reset_presenter,
                                        event);
      if (FactoryResetPresenter_IsComplete(
              g_router_state.factory_reset_presenter)) {
        Router_GoToRoute(ROUTE_MENU);
      }
    }
  }
  /* Other routes (INIT, RUNNING) have no specific interactions yet */
}

/**
 * @brief Periodic tick for current route (animations, updates, etc.)
 */
void Router_OnTick(uint32_t current_tick) {
  SystemState_t sysState = Router_GetSystemState();

  /* State Machine Driven Routing */
  RouteTypeDef targetRoute = g_router_state.current_route;

  /* Debug: Print state transitions */
  static SystemState_t prev_state = STATE_INIT;
  if (sysState != prev_state) {
    printf("Router: System state changed from %d to %d\n", prev_state,
           sysState);
    prev_state = sysState;
  }

  switch (sysState) {
  case STATE_INIT:
    targetRoute = ROUTE_INIT;
    break;
  case STATE_COD:
    /* Start with DATE_TIME configuration, then move to SCHEDULE */
    if (g_router_state.current_route != ROUTE_DATE_TIME &&
        g_router_state.current_route != ROUTE_CHANGE_SCHEDULE) {
      targetRoute = ROUTE_DATE_TIME;
    }
    break;
  case STATE_NOT_INST:
    targetRoute = ROUTE_NOT_INST;
    break;
  case STATE_ADAPT:
    targetRoute = ROUTE_ADAPT;
    break;
  case STATE_ADAPT_FAIL:
    targetRoute = ROUTE_ADAPT_FAIL;
    break;
  case STATE_FACTORY_RST:
    targetRoute = ROUTE_FACTORY_RESET;
    break;
  case STATE_RUNNING:
    /* Allow Menu and sub-menus in Running state */
    if (g_router_state.current_route != ROUTE_MENU &&
        g_router_state.current_route != ROUTE_BOOST &&
        g_router_state.current_route != ROUTE_EDIT_TEMP_OFFSET &&
        g_router_state.current_route != ROUTE_CHANGE_SCHEDULE &&
        g_router_state.current_route != ROUTE_FACTORY_RESET) {
      targetRoute = ROUTE_HOME;
    }
    break;
  default:
    break;
  }

  /* Perform route transition if needed */
  if (targetRoute != g_router_state.current_route) {
    printf("Router: Route transition from %d to %d\n",
           g_router_state.current_route, targetRoute);
    Router_GoToRoute(targetRoute);
  }

  /* Update current view based on route */
  if (g_router_state.current_route == ROUTE_DATE_TIME) {
    if (g_router_state.dt_presenter) {
      // SetDateTimePresenter_Run(g_router_state.dt_presenter);
    }
  } else if (g_router_state.current_route == ROUTE_CHANGE_SCHEDULE) {
    /* No periodic run needed for now */
  } else if (g_router_state.current_route == ROUTE_INIT) {
    if (g_router_state.loading_presenter) {
      LoadingPresenter_Run(g_router_state.loading_presenter, current_tick);
    }
  } else if (g_router_state.current_route == ROUTE_NOT_INST) {
    if (g_router_state.waiting_presenter) {
      WaitingPresenter_Run(g_router_state.waiting_presenter);
    }
  } else if (g_router_state.current_route == ROUTE_ADAPT) {
    if (g_router_state.adapt_presenter) {
      LoadingPresenter_Run(g_router_state.adapt_presenter, current_tick);
    }
  } else if (g_router_state.current_route == ROUTE_ADAPT_FAIL) {
    if (g_router_state.adapt_fail_presenter) {
      WaitingPresenter_Run(g_router_state.adapt_fail_presenter);
    }
  } else if (g_router_state.current_route == ROUTE_RUNNING) {
    if (g_router_state.loading_presenter) {
      LoadingPresenter_Run(g_router_state.loading_presenter, current_tick);
    }
  } else if (g_router_state.current_route == ROUTE_HOME) {
    if (g_router_state.home_presenter) {
      HomePresenter_Run(g_router_state.home_presenter, current_tick);
    }
  } else if (g_router_state.current_route == ROUTE_BOOST) {
    if (g_router_state.boost_presenter) {
      BoostPresenter_Run(g_router_state.boost_presenter, current_tick);
    }
  } else if (g_router_state.current_route == ROUTE_MENU) {
    if (g_router_state.menu_presenter) {
      MenuPresenter_Run(g_router_state.menu_presenter, current_tick);
    }
  } else if (g_router_state.current_route == ROUTE_EDIT_TEMP_OFFSET) {
    /* SetTempOffsetPresenter doesn't have Run, it updates on event */
  } else if (g_router_state.current_route == ROUTE_FACTORY_RESET) {
    if (g_router_state.factory_reset_presenter) {
      FactoryResetPresenter_Run(g_router_state.factory_reset_presenter,
                                current_tick);
    }
  }
}

/**
 * @brief Explicitly change route
 */
void Router_GoToRoute(RouteTypeDef route) {
  if (g_router_state.current_route == route)
    return;

  /* Initialize new route */
  if (route == ROUTE_DATE_TIME) {
    if (!g_router_state.dt_view) {
      g_router_state.dt_view = SetDateTimeView_Init(false, 2026);
    }
    if (g_router_state.dt_view && !g_router_state.dt_presenter) {
      g_router_state.dt_presenter =
          SetDateTimePresenter_Init(g_router_state.dt_view, 2026);
      /* Run immediately after initializing to ensure screen is displayed */
      // SetDateTimePresenter_Run(g_router_state.dt_presenter);
    }
  } else if (route == ROUTE_CHANGE_SCHEDULE) {
    if (!g_router_state.sch_view) {
      g_router_state.sch_view = ChangeScheduleView_Init();
    }
    if (g_router_state.sch_view && !g_router_state.sch_presenter) {
      /* If coming from Menu (Running state), skip confirmation */
      bool skip_confirmation = (Router_GetSystemState() == STATE_RUNNING);
      g_router_state.sch_presenter = ChangeSchedulePresenter_Init(
          g_router_state.sch_view, g_router_state.config_access,
          skip_confirmation);
    }
  } else if (route == ROUTE_INIT) {
    if (!g_router_state.loading_view) {
      g_router_state.loading_view =
          LoadingView_Init("Initialization", LV_ALIGN_LEFT_MID, 10);
    }
    if (g_router_state.loading_view && !g_router_state.loading_presenter) {
      g_router_state.loading_presenter =
          LoadingPresenter_Init(g_router_state.loading_view);
      LoadingPresenter_Run(g_router_state.loading_presenter,
                           osKernelGetTickCount());
    }
  } else if (route == ROUTE_NOT_INST) {
    if (!g_router_state.waiting_view) {
      g_router_state.waiting_view =
          WaitingView_Init("Begin\nInstallation?", -5);
    }
    if (g_router_state.waiting_view && !g_router_state.waiting_presenter) {
      g_router_state.waiting_presenter =
          WaitingPresenter_Init(g_router_state.waiting_view);
      WaitingPresenter_Run(g_router_state.waiting_presenter);
    } else if (g_router_state.waiting_presenter) {
      /* Reset completion flag when re-entering the route */
      WaitingPresenter_Reset(g_router_state.waiting_presenter);
      WaitingPresenter_Run(g_router_state.waiting_presenter);
    }
  } else if (route == ROUTE_ADAPT) {
    if (!g_router_state.adapt_view) {
      g_router_state.adapt_view =
          LoadingView_Init("Adaptation", LV_ALIGN_LEFT_MID, 20);
    }
    if (g_router_state.adapt_view && !g_router_state.adapt_presenter) {
      g_router_state.adapt_presenter =
          LoadingPresenter_Init(g_router_state.adapt_view);
      LoadingPresenter_Run(g_router_state.adapt_presenter,
                           osKernelGetTickCount());
    }
  } else if (route == ROUTE_ADAPT_FAIL) {
    if (!g_router_state.adapt_fail_view) {
      g_router_state.adapt_fail_view =
          WaitingView_Init("Adaptation\nFailed!", -5);
    }
    if (g_router_state.adapt_fail_view &&
        !g_router_state.adapt_fail_presenter) {
      g_router_state.adapt_fail_presenter =
          WaitingPresenter_Init(g_router_state.adapt_fail_view);
      WaitingPresenter_Run(g_router_state.adapt_fail_presenter);
    } else if (g_router_state.adapt_fail_presenter) {
      /* Reset completion flag when re-entering the route */
      WaitingPresenter_Reset(g_router_state.adapt_fail_presenter);
      WaitingPresenter_Run(g_router_state.adapt_fail_presenter);
    }
  } else if (route == ROUTE_RUNNING) {
    if (!g_router_state.loading_view) {
      g_router_state.loading_view =
          LoadingView_Init("Running", LV_ALIGN_LEFT_MID, 25);
    }
    if (g_router_state.loading_view && !g_router_state.loading_presenter) {
      g_router_state.loading_presenter =
          LoadingPresenter_Init(g_router_state.loading_view);
      LoadingPresenter_Run(g_router_state.loading_presenter,
                           osKernelGetTickCount());
    }
  } else if (route == ROUTE_HOME) {
    if (!g_router_state.home_view) {
      g_router_state.home_view = HomeView_Init();
    }
    if (g_router_state.home_view && !g_router_state.home_presenter) {
      g_router_state.home_presenter = HomePresenter_Init(
          g_router_state.home_view, g_router_state.system_context,
          g_router_state.config_access, g_router_state.sensor_values_access);
    }
  } else if (route == ROUTE_BOOST) {
    if (!g_router_state.boost_view) {
      g_router_state.boost_view = BoostView_Init();
    }
    if (g_router_state.boost_view && !g_router_state.boost_presenter) {
      g_router_state.boost_presenter = BoostPresenter_Init(
          g_router_state.boost_view, g_router_state.system_context);
    }
  } else if (route == ROUTE_MENU) {
    if (!g_router_state.menu_view) {
      g_router_state.menu_view =
          MenuView_Init("Edit temp offset\nEdit schedule");
    }
    if (g_router_state.menu_view && !g_router_state.menu_presenter) {
      g_router_state.menu_presenter = MenuPresenter_Init(
          g_router_state.menu_view, g_router_state.system_context,
          g_router_state.config_access, g_router_state.sensor_values_access);
    }
  } else if (route == ROUTE_EDIT_TEMP_OFFSET) {
    if (!g_router_state.temp_offset_view) {
      /* View is initialized by presenter, but we need to create it first */
      /* We pass NULL options here, presenter will set them */
      g_router_state.temp_offset_view = SetValueView_Init(NULL, NULL, NULL);
    }
    if (g_router_state.temp_offset_view &&
        !g_router_state.temp_offset_presenter) {
      g_router_state.temp_offset_presenter = SetTempOffsetPresenter_Init(
          g_router_state.temp_offset_view, g_router_state.config_access);
    }
  } else if (route == ROUTE_FACTORY_RESET) {
    if (!g_router_state.factory_reset_presenter) {
      g_router_state.factory_reset_presenter =
          FactoryResetPresenter_Init(g_router_state.vp2system_queue);
    }
  }

  /* Cleanup old route */
  switch (g_router_state.current_route) {
  case ROUTE_DATE_TIME:
    if (g_router_state.dt_presenter) {
      SetDateTimePresenter_Deinit(g_router_state.dt_presenter);
      g_router_state.dt_presenter = NULL;
    }
    if (g_router_state.dt_view) {
      SetDateTimeView_Deinit(g_router_state.dt_view);
      g_router_state.dt_view = NULL;
    }
    break;
  case ROUTE_CHANGE_SCHEDULE:
    if (g_router_state.sch_presenter) {
      ChangeSchedulePresenter_Deinit(g_router_state.sch_presenter);
      g_router_state.sch_presenter = NULL;
    }
    if (g_router_state.sch_view) {
      ChangeScheduleView_Deinit(g_router_state.sch_view);
      g_router_state.sch_view = NULL;
    }
    break;
  case ROUTE_INIT:
  case ROUTE_RUNNING:
    if (g_router_state.loading_presenter) {
      LoadingPresenter_Deinit(g_router_state.loading_presenter);
      g_router_state.loading_presenter = NULL;
    }
    if (g_router_state.loading_view) {
      LoadingView_Deinit(g_router_state.loading_view);
      g_router_state.loading_view = NULL;
    }
    break;
  case ROUTE_NOT_INST:
    if (g_router_state.waiting_presenter) {
      WaitingPresenter_Deinit(g_router_state.waiting_presenter);
      g_router_state.waiting_presenter = NULL;
    }
    if (g_router_state.waiting_view) {
      WaitingView_Deinit(g_router_state.waiting_view);
      g_router_state.waiting_view = NULL;
    }
    break;
  case ROUTE_ADAPT:
    if (g_router_state.adapt_presenter) {
      LoadingPresenter_Deinit(g_router_state.adapt_presenter);
      g_router_state.adapt_presenter = NULL;
    }
    if (g_router_state.adapt_view) {
      LoadingView_Deinit(g_router_state.adapt_view);
      g_router_state.adapt_view = NULL;
    }
    break;
  case ROUTE_ADAPT_FAIL:
    if (g_router_state.adapt_fail_presenter) {
      WaitingPresenter_Deinit(g_router_state.adapt_fail_presenter);
      g_router_state.adapt_fail_presenter = NULL;
    }
    if (g_router_state.adapt_fail_view) {
      WaitingView_Deinit(g_router_state.adapt_fail_view);
      g_router_state.adapt_fail_view = NULL;
    }
    break;
  case ROUTE_HOME:
    if (g_router_state.home_presenter) {
      HomePresenter_Deinit(g_router_state.home_presenter);
      g_router_state.home_presenter = NULL;
    }
    if (g_router_state.home_view) {
      HomeView_Deinit(g_router_state.home_view);
      g_router_state.home_view = NULL;
    }
    break;
  case ROUTE_BOOST:
    if (g_router_state.boost_presenter) {
      BoostPresenter_Deinit(g_router_state.boost_presenter);
      g_router_state.boost_presenter = NULL;
    }
    if (g_router_state.boost_view) {
      BoostView_Deinit(g_router_state.boost_view);
      g_router_state.boost_view = NULL;
    }
    break;
  case ROUTE_MENU:
    if (g_router_state.menu_presenter) {
      MenuPresenter_Deinit(g_router_state.menu_presenter);
      g_router_state.menu_presenter = NULL;
    }
    if (g_router_state.menu_view) {
      MenuView_Deinit(g_router_state.menu_view);
      g_router_state.menu_view = NULL;
    }
    break;
  case ROUTE_EDIT_TEMP_OFFSET:
    if (g_router_state.temp_offset_presenter) {
      SetTempOffsetPresenter_Deinit(g_router_state.temp_offset_presenter);
      g_router_state.temp_offset_presenter = NULL;
    }
    if (g_router_state.temp_offset_view) {
      SetValueView_Deinit(g_router_state.temp_offset_view);
      g_router_state.temp_offset_view = NULL;
    }
    break;
  case ROUTE_FACTORY_RESET:
    if (g_router_state.factory_reset_presenter) {
      FactoryResetPresenter_Deinit(g_router_state.factory_reset_presenter);
      g_router_state.factory_reset_presenter = NULL;
    }
    break;
  default:
    break;
  }

  /* Switch to new route */
  g_router_state.current_route = route;
}

/**
 * @brief Get current route
 */
RouteTypeDef Router_GetCurrentRoute(void) {
  return g_router_state.current_route;
}
