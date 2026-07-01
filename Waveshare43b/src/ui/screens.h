#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SERVICE = 2,
    _SCREEN_ID_LAST = 2
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *service;
    lv_obj_t *lbl_1;
    lv_obj_t *btn_1;
    lv_obj_t *lbl_btn_1;
    lv_obj_t *drpdown_1;
    lv_obj_t *btn_2;
    lv_obj_t *lbl_btn_2;
    lv_obj_t *btn_3;
    lv_obj_t *lbl_btn_3;
    lv_obj_t *btn_4;
    lv_obj_t *lbl_btn_4;
    lv_obj_t *btn_5;
    lv_obj_t *lbl_btn_5;
    lv_obj_t *btn_6;
    lv_obj_t *lbl_btn_6;
    lv_obj_t *btn_service;
    lv_obj_t *lbl_btn_service;
    lv_obj_t *txt_1;
    lv_obj_t *key_1;
    lv_obj_t *btn_main;
    lv_obj_t *lbl_btn_main;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_service();
void tick_screen_service();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/