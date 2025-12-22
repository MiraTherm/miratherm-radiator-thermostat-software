#include "set_date_time_view.h"
#include <stdlib.h>

typedef struct SetDateTimeView
{
    SetDateView_t *date_view;
    SetTimeView_t *time_view;
    SetDstView_t *dst_view;
} SetDateTimeView_t;

SetDateTimeView_t* SetDateTimeView_Init(void)
{
    SetDateTimeView_t *view = (SetDateTimeView_t *)malloc(sizeof(SetDateTimeView_t));
    if (!view)
        return NULL;

    view->date_view = SetDateView_Init();
    view->time_view = SetTimeView_Init();
    view->dst_view = SetDstView_Init();

    if (!view->date_view || !view->time_view || !view->dst_view)
    {
        SetDateTimeView_Deinit(view);
        return NULL;
    }

    return view;
}

void SetDateTimeView_Deinit(SetDateTimeView_t *view)
{
    if (view)
    {
        if (view->date_view) SetDateView_Deinit(view->date_view);
        if (view->time_view) SetTimeView_Deinit(view->time_view);
        if (view->dst_view) SetDstView_Deinit(view->dst_view);
        free(view);
    }
}

SetDateView_t* SetDateTimeView_GetDateView(SetDateTimeView_t *view)
{
    return view ? view->date_view : NULL;
}

SetTimeView_t* SetDateTimeView_GetTimeView(SetDateTimeView_t *view)
{
    return view ? view->time_view : NULL;
}

SetDstView_t* SetDateTimeView_GetDstView(SetDateTimeView_t *view)
{
    return view ? view->dst_view : NULL;
}
