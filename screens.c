#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

//
// Screens
//

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // lbl_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_1 = obj;
            lv_obj_set_pos(obj, 0, 224);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "CYD_RS485");
        }
        {
            // btn_1
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.btn_1 = obj;
            lv_obj_set_pos(obj, 4, 51);
            lv_obj_set_size(obj, 97, 58);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xa021f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21f329), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // lbl_btn_1
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lbl_btn_1 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "1");
                }
            }
        }
        {
            // drpdown_1
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            objects.drpdown_1 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_dropdown_set_options_static(obj, "Option 1\nOption 2\nOption 3");
            lv_dropdown_set_selected(obj, 0);
        }
        {
            // btn_2
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.btn_2 = obj;
            lv_obj_set_pos(obj, 110, 51);
            lv_obj_set_size(obj, 99, 58);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xa021f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // lbl_btn_2
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lbl_btn_2 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "2");
                }
            }
        }
        {
            // btn_3
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.btn_3 = obj;
            lv_obj_set_pos(obj, 217, 51);
            lv_obj_set_size(obj, 97, 58);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xa021f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xa021f3), LV_PART_MAIN | LV_STATE_FOCUSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // lbl_btn_3
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lbl_btn_3 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "3");
                }
            }
        }
        {
            // btn_4
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.btn_4 = obj;
            lv_obj_set_pos(obj, 4, 120);
            lv_obj_set_size(obj, 97, 58);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xa021f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // lbl_btn_4
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lbl_btn_4 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "4");
                }
            }
        }
        {
            // btn_5
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.btn_5 = obj;
            lv_obj_set_pos(obj, 110, 120);
            lv_obj_set_size(obj, 99, 58);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xa021f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // lbl_btn_5
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lbl_btn_5 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "5");
                }
            }
        }
        {
            // btn_6
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.btn_6 = obj;
            lv_obj_set_pos(obj, 217, 120);
            lv_obj_set_size(obj, 97, 58);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xa021f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // lbl_btn_6
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lbl_btn_6 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "6");
                }
            }
        }
        {
            // btn_service
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.btn_service = obj;
            lv_obj_set_pos(obj, 266, 220);
            lv_obj_set_size(obj, 54, 20);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // lbl_btn_service
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lbl_btn_service = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "service");
                }
            }
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
}

void create_screen_service() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.service = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // txt_1
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.txt_1 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 320, 51);
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_placeholder_text(obj, "Device ID des Geraets eingeben");
            lv_textarea_set_one_line(obj, false);
            lv_textarea_set_password_mode(obj, false);
        }
        {
            // Key_1
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.key_1 = obj;
            lv_obj_set_pos(obj, -5, 57);
            lv_obj_set_size(obj, 325, 126);
            lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_NUMBER);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // btn_main
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.btn_main = obj;
            lv_obj_set_pos(obj, 266, 221);
            lv_obj_set_size(obj, 54, 20);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // lbl_btn_main
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lbl_btn_main = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Main");
                }
            }
        }
    }
    
    tick_screen_service();
}

void tick_screen_service() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_service,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 2) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_main();
    create_screen_service();
}