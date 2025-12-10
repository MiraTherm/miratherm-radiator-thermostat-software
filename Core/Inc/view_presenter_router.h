#ifndef CORE_INC_VIEW_PRESENTER_ROUTER_H
#define CORE_INC_VIEW_PRESENTER_ROUTER_H

#include "input_task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Router for MVP pattern - manages which view/presenter is active
 */

typedef enum
{
    ROUTE_DATE_TIME = 0,
    ROUTE_INSTALLATION = 1,
} RouteTypeDef;

/**
 * @brief Initialize the router and activate initial route (DATE_TIME)
 */
void Router_Init(void);

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
