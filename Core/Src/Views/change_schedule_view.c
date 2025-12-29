#include "change_schedule_view.h"
#include <stdlib.h>

typedef struct ChangeScheduleView
{
    SetBoolView_t *bool_view;
    SetValueView_t *value_view;
    SetTimeSlotView_t *time_slot_view;
} ChangeScheduleView_t;

ChangeScheduleView_t* ChangeScheduleView_Init(void)
{
    ChangeScheduleView_t *view = (ChangeScheduleView_t *)malloc(sizeof(ChangeScheduleView_t));
    if (!view) return NULL;

    /* Initialize sub-views. They are created but not shown yet. */
    /* Note: We initialize them with default/placeholder values, they will be updated by presenter */
    
    view->bool_view = SetBoolView_Init("Edit schedule?", "Yes", "No", true);
    view->value_view = SetValueView_Init("Set value", "", NULL);
    view->time_slot_view = SetTimeSlotView_Init("Set time slot");

    if (!view->bool_view || !view->value_view || !view->time_slot_view)
    {
        ChangeScheduleView_Deinit(view);
        return NULL;
    }

    return view;
}

void ChangeScheduleView_Deinit(ChangeScheduleView_t *view)
{
    if (view)
    {
        if (view->bool_view) SetBoolView_Deinit(view->bool_view);
        if (view->value_view) SetValueView_Deinit(view->value_view);
        if (view->time_slot_view) SetTimeSlotView_Deinit(view->time_slot_view);
        free(view);
    }
}

SetBoolView_t* ChangeScheduleView_GetBoolView(ChangeScheduleView_t *view)
{
    return view ? view->bool_view : NULL;
}

SetValueView_t* ChangeScheduleView_GetValueView(ChangeScheduleView_t *view)
{
    return view ? view->value_view : NULL;
}

SetTimeSlotView_t* ChangeScheduleView_GetTimeSlotView(ChangeScheduleView_t *view)
{
    return view ? view->time_slot_view : NULL;
}
