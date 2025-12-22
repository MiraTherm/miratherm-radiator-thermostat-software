#include "set_dst_presenter.h"
#include "set_dst_view.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct SetDstPresenter
{
    SetDstView_t *view;
    SetDst_ViewModelData_t data;
    bool is_complete;
} SetDstPresenter_t;

SetDstPresenter_t* SetDstPresenter_Init(SetDstView_t *view)
{
    SetDstPresenter_t *presenter = (SetDstPresenter_t *)malloc(sizeof(SetDstPresenter_t));
    if (!presenter)
        return NULL;

    presenter->view = view;
    presenter->is_complete = false;
    presenter->data.is_summer_time = false;

    return presenter;
}

void SetDstPresenter_Deinit(SetDstPresenter_t *presenter)
{
    if (presenter)
        free(presenter);
}

void SetDstPresenter_HandleEvent(SetDstPresenter_t *presenter, const Input2VPEvent_t *event)
{
    if (!presenter || !event)
        return;

    bool state_changed = false;

    if (event->type == EVT_CTRL_WHEEL_DELTA)
    {
        if (event->delta < 0)
        {
            presenter->data.is_summer_time = false;
        }
        else if (event->delta > 0)
        {
            presenter->data.is_summer_time = true;
        }
        state_changed = true;
    }
    else if ((event->type == EVT_CENTRAL_BTN || event->type == EVT_CENTRAL_DOUBLE_CLICK) && event->button_action == BUTTON_ACTION_PRESSED)
    {
        presenter->is_complete = true;
        state_changed = true;
    }

    if (state_changed && presenter->view)
    {
        SetDstView_Render(presenter->view, &presenter->data);
    }
}

void SetDstPresenter_Reset(SetDstPresenter_t *presenter)
{
    if (presenter)
    {
        presenter->is_complete = false;
    }
}

bool SetDstPresenter_IsComplete(SetDstPresenter_t *presenter)
{
    if (!presenter)
        return false;
    return presenter->is_complete;
}

const SetDst_ViewModelData_t* SetDstPresenter_GetData(SetDstPresenter_t *presenter)
{
    if (!presenter)
        return NULL;
    return &presenter->data;
}
