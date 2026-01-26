#ifndef CORE_INC_VIEWS_MENU_VIEW_H
#define CORE_INC_VIEWS_MENU_VIEW_H

#include "lvgl/lvgl.h"
#include "menu_viewmodel.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MenuView MenuView_t;

MenuView_t* MenuView_Init(const char *options);
void MenuView_Deinit(MenuView_t *view);
void MenuView_Render(MenuView_t *view, const Menu_ViewModelData_t *model);
void MenuView_Show(MenuView_t *view);
void MenuView_Hide(MenuView_t *view);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_MENU_VIEW_H */
