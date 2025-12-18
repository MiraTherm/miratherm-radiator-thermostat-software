#ifndef CORE_INC_VIEW_PRESENTER_ROUTER_H
#define CORE_INC_VIEW_PRESENTER_ROUTER_H

#include "input_task.h"
#include "system_task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Router for MVP pattern - manages which view/presenter is active
 */

typedef enum
{
    ROUTE_INIT = 0,
    ROUTE_DATE_TIME,
    ROUTE_NOT_INST,
    ROUTE_ADAPT,
    ROUTE_ADAPT_FAIL,
    ROUTE_RUNNING,
} RouteTypeDef;

/**
 * @brief Initialize the router and activate initial route (DATE_TIME)
 * @param vp2system_queue Queue to send events to System Task
 * @param system_context Shared system context
 */
void Router_Init(osMessageQueueId_t vp2system_queue, SystemContextAccessTypeDef *system_context);

/**
 * @brief Deinitialize the router
 */
void Router_Deinit(void);

/**
 * @brief Handle input event in current route
 */
void Router_HandleEvent(const Input2VPEvent_t *event);

/**
 * @brief Periodic tick for current route (animations, updates, etc.)
 */
void Router_OnTick(uint32_t current_tick);

/**
 * @brief Explicitly change route
 */
void Router_GoToRoute(RouteTypeDef route);

/**
 * @brief Get current route
 */
RouteTypeDef Router_GetCurrentRoute(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEW_PRESENTER_ROUTER_H */
