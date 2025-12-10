#include "view_presenter_router.h"
#include "date_time_presenter.h"
#include "date_time_view.h"
#include "intallation_presenter.h"
#include "installation_view.h"
#if DEBUG_LEDS
#include "stm32wbxx_nucleo.h"
#endif
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Global router state
 */
typedef struct
{
    RouteTypeDef current_route;
    
    /* Date/Time route */
    DateTimePresenter_t *dt_presenter;
    DateTimeView_t *dt_view;
    
    /* Installation route */
    InstallationPresenter_t *inst_presenter;
    InstallationView_t *inst_view;
} Router_State_t;

static Router_State_t g_router_state = {
    .current_route = ROUTE_DATE_TIME,
    .dt_presenter = NULL,
    .dt_view = NULL,
    .inst_presenter = NULL,
    .inst_view = NULL,
};

#if DEBUG_LEDS
static void Router_UpdateDebugLeds(const Input2VPEvent_t *event)
{
    if (!event)
        return;

    uint16_t target_led = 0;
    bool should_update = true;
    switch (event->type)
    {
        case EVT_MODE_BTN:
            target_led = LED3;
            break;
        case EVT_CENTRAL_BTN:
        case EVT_CENTRAL_DOUBLE_CLICK:
            target_led = LED2;
            break;
        case EVT_MENU_BTN:
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
}
#endif

/**
 * @brief Initialize the router and activate initial route (DATE_TIME)
 */
void Router_Init(void)
{
    /* Initialize date/time presenter and view */
    g_router_state.dt_presenter = DateTimePresenter_Init();
    if (g_router_state.dt_presenter)
    {
        g_router_state.dt_view = DateTimeView_Init(g_router_state.dt_presenter);
    }

    /* Initialize installation presenter and view (but don't show yet) */
    g_router_state.inst_presenter = InstallationPresenter_Init();
    
    /* Set initial route */
    g_router_state.current_route = ROUTE_DATE_TIME;
}

/**
 * @brief Deinitialize the router
 */
void Router_Deinit(void)
{
    if (g_router_state.dt_view)
    {
        DateTimeView_Deinit(g_router_state.dt_view);
        g_router_state.dt_view = NULL;
    }

    if (g_router_state.dt_presenter)
    {
        DateTimePresenter_Deinit(g_router_state.dt_presenter);
        g_router_state.dt_presenter = NULL;
    }

    if (g_router_state.inst_view)
    {
        InstallationView_Deinit(g_router_state.inst_view);
        g_router_state.inst_view = NULL;
    }

    if (g_router_state.inst_presenter)
    {
        InstallationPresenter_Deinit(g_router_state.inst_presenter);
        g_router_state.inst_presenter = NULL;
    }
}

/**
 * @brief Handle input event in current route
 */
void Router_HandleEvent(const Input2VPEvent_t *event)
{
    if (!event)
        return;

#if DEBUG_LEDS
    Router_UpdateDebugLeds(event);
#endif

    if (g_router_state.current_route == ROUTE_DATE_TIME)
    {
        if (g_router_state.dt_presenter)
        {
            DateTimePresenter_HandleEvent(g_router_state.dt_presenter, event);
            
            /* Check if date/time setup is complete and transition to installation */
            if (DateTimePresenter_IsComplete(g_router_state.dt_presenter))
            {
                Router_GoToRoute(ROUTE_INSTALLATION);
            }
            else
            {
                /* Update view for current page */
                if (g_router_state.dt_view)
                {
                    DateTimeView_Render(g_router_state.dt_view);
                }
            }
        }
    }
    else if (g_router_state.current_route == ROUTE_INSTALLATION)
    {
        /* Installation view is read-only, no input handling needed */
        (void)event;
    }
}

/**
 * @brief Periodic tick for current route (animations, updates, etc.)
 */
void Router_OnTick(uint32_t current_tick)
{
    (void)current_tick;

    if (g_router_state.current_route == ROUTE_DATE_TIME)
    {
        /* Update date/time view if needed */
        if (g_router_state.dt_view)
        {
            DateTimeView_Render(g_router_state.dt_view);
        }
    }
    else if (g_router_state.current_route == ROUTE_INSTALLATION)
    {
        /* Update installation view with animation */
        if (!g_router_state.inst_view && g_router_state.inst_presenter)
        {
            g_router_state.inst_view = InstallationView_Init(g_router_state.inst_presenter);
        }
        
        if (g_router_state.inst_view)
        {
            InstallationView_Render(g_router_state.inst_view);
        }
    }
}

/**
 * @brief Explicitly change route
 */
void Router_GoToRoute(RouteTypeDef route)
{
    if (g_router_state.current_route == route)
        return;

    /* Cleanup current route views */
    if (g_router_state.current_route == ROUTE_DATE_TIME)
    {
        if (g_router_state.dt_view)
        {
            DateTimeView_Deinit(g_router_state.dt_view);
            g_router_state.dt_view = NULL;
        }
    }
    else if (g_router_state.current_route == ROUTE_INSTALLATION)
    {
        if (g_router_state.inst_view)
        {
            InstallationView_Deinit(g_router_state.inst_view);
            g_router_state.inst_view = NULL;
        }
    }

    /* Switch to new route */
    g_router_state.current_route = route;

    if (route == ROUTE_DATE_TIME)
    {
        if (g_router_state.dt_presenter && !g_router_state.dt_view)
        {
            g_router_state.dt_view = DateTimeView_Init(g_router_state.dt_presenter);
        }
    }
    else if (route == ROUTE_INSTALLATION)
    {
        if (g_router_state.inst_presenter && !g_router_state.inst_view)
        {
            g_router_state.inst_view = InstallationView_Init(g_router_state.inst_presenter);
        }
    }
}

/**
 * @brief Get current route
 */
RouteTypeDef Router_GetCurrentRoute(void)
{
    return g_router_state.current_route;
}
