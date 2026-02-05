/**
 ******************************************************************************
 * @file           :  view_presenter_router.h
 * @brief          :  Model-View-Presenter (MVP) routing and navigation
 *
 * @details        :  Manages view/presenter lifecycle and state-driven routing
 *                    for the user interface. Coordinates route transitions based
 *                    on system state changes and handles input event routing to
 *                    the active view/presenter pair.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */

#ifndef CORE_INC_VIEW_PRESENTER_ROUTER_H
#define CORE_INC_VIEW_PRESENTER_ROUTER_H

#include "input_task.h"
#include "sensor_task.h"
#include "storage_task.h"
#include "system_task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef RouteTypeDef
 * @brief User interface route/screen enumeration
 * @details Defines all possible screens and their associated view/presenter pairs:
 *          - ROUTE_INIT: Initialization screen
 *          - ROUTE_DATE_TIME: Date/time configuration screen
 *          - ROUTE_CHANGE_SCHEDULE: Daily schedule configuration screen
 *          - ROUTE_NOT_INST: "Not installed" / awaiting installation
 *          - ROUTE_ADAPT: Radiator adaptation in progress
 *          - ROUTE_ADAPT_FAIL: Adaptation failed, retry prompt
 *          - ROUTE_RUNNING: Normal operation (home screen parent)
 *          - ROUTE_HOME: Home/main screen with current temperature
 *          - ROUTE_BOOST: Boost mode activation screen
 *          - ROUTE_MENU: Settings menu
 *          - ROUTE_EDIT_TEMP_OFFSET: Temperature sensor offset calibration
 *          - ROUTE_FACTORY_RESET: Factory reset confirmation
 */
typedef enum {
  ROUTE_INIT = 0,           /**< Startup initialization screen */
  ROUTE_DATE_TIME,          /**< Date/time configuration */
  ROUTE_CHANGE_SCHEDULE,    /**< Schedule configuration */
  ROUTE_NOT_INST,           /**< Not installed - awaiting installation */
  ROUTE_ADAPT,              /**< Adaptation in progress */
  ROUTE_ADAPT_FAIL,         /**< Adaptation failed - retry prompt */
  ROUTE_RUNNING,            /**< Running state (parent route) */
  ROUTE_HOME,               /**< Home/main screen */
  ROUTE_BOOST,              /**< Boost mode screen */
  ROUTE_MENU,               /**< Settings menu */
  ROUTE_EDIT_TEMP_OFFSET,   /**< Temperature offset calibration */
  ROUTE_FACTORY_RESET,      /**< Factory reset confirmation */
} RouteTypeDef;

/**
 * @brief Initialize the MVP router
 * @details Sets up initial route and registers all queue handles and data
 *          access pointers. Initializes the first screen view/presenter pair.
 * @param vp2system_queue Queue for sending user actions to system task
 * @param system_context Shared system state context (for reading state)
 * @param config_access Shared configuration data access
 * @param sensor_values_access Shared sensor measurement values
 * @return void; calls Error_Handler() if queues or contexts are invalid
 * @note Must be called once during VP task startup
 * @see ViewPresenterTaskArgsTypeDef, RouteTypeDef
 */
void Router_Init(osMessageQueueId_t vp2system_queue,
                 SystemModel_t *system_context,
                 ConfigModel_t *config_access,
                 SensorModel_t *sensor_values_access);

/**
 * @brief Deinitialize the router
 * @details Cleans up all active view/presenter pairs and releases resources.
 *          Should be called when the UI task is shutting down.
 * @return void
 * @note Safe to call even if Router_Init was not called
 * @see Router_Init
 */
void Router_Deinit(void);

/**
 * @brief Process input event in the current route
 * @details Routes the input event to the active view/presenter for handling.
 *          Events may trigger route transitions or presenter state changes.
 * @param event Pointer to input event (button, encoder movement)
 * @return void
 * @note Safe to call with NULL event (no-op)
 * @see Input2VPEvent_t, Router_GoToRoute
 */
void Router_HandleEvent(const Input2VPEvent_t *event);

/**
 * @brief Perform periodic UI updates
 * @details Called regularly to update animations, check for state transitions,
 *          and run periodic presenter updates. Monitors system state and
 *          automatically transitions routes as needed. Should be called
 *          from the UI task main loop.
 * @param current_tick Current FreeRTOS tick count for animation timing
 * @return void
 * @note Performs state-driven routing: automatically transitions to the
 *        appropriate route based on current system state
 * @see Router_GoToRoute, SystemState_t
 */
void Router_OnTick(uint32_t current_tick);

/**
 * @brief Explicitly change to a different route
 * @details Performs route transition: cleans up old view/presenter, initializes
 *          new view/presenter. Called automatically by Router_OnTick when
 *          system state changes, or can be called explicitly.
 * @param route Target route to navigate to
 * @return void (no-op if already on target route)
 * @note Handles view lifecycle (init/deinit) automatically
 * @see RouteTypeDef, Router_OnTick
 */
void Router_GoToRoute(RouteTypeDef route);

/**
 * @brief Query current active route
 * @details Returns the currently active route without any locking.
 *          Used for internal state tracking and debugging.
 * @return Current RouteTypeDef
 * @note Non-blocking; does not acquire any mutexes
 * @see RouteTypeDef
 */
RouteTypeDef Router_GetCurrentRoute(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEW_PRESENTER_ROUTER_H */
