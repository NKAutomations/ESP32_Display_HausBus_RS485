#include "hausbus_app.h"

#include <Arduino.h>
#include <Preferences.h>

#include "hausbus_kernal.h"
#include "lvgl_v8_port.h"
#include "ui/ui.h"
#include "ui/screens.h"

static Preferences prefs;

static const int RS485_RX = 43;
static const int RS485_TX = 44;
static const uint32_t RS485_BAUD = 57600;

static const uint16_t BTN_INSTANCE_BASE = 17;
static const uint16_t LED_INSTANCE_BASE = 49;
static const uint8_t UI_BUTTON_COUNT = 6;

static String dropdown_options[3] = {"Ebene1", "Ebene2", "Ebene3"};
static const uint32_t DEVICE_IDS_DEFAULT[3] = {9061, 9062, 9063};
static uint32_t DEVICE_IDS[3] = {9061, 9062, 9063};
static const char *NVS_KEY_ID[3] = {"devid_0", "devid_1", "devid_2"};

static const uint32_t COLOR_BTN_DEFAULT = 0xA021F3;
static const uint32_t COLOR_BTN_PRESSED_LOC = 0x00C853;
static const uint32_t COLOR_BTN_PRESSED_REM = 0xFFA500;
static const uint32_t COLOR_BORDER_DEFAULT = 0x6A00B8;
static const uint32_t COLOR_BORDER_PRESSED = 0x009624;

static const char *PING_FUNCTION = "OUT";
static const uint16_t PING_INSTANCE_ID = 210;
static const bool PING_REPLY_USE_REQUEST_INSTANCE = true;
static const char *PING_GET_ACTION = "gSTATUS";
static const char *PING_REPLY_ACTION = "rSTATUS";
static const char *PING_REPLY_VALUE = "1";
static const uint32_t PING_REPLY_DELAY_MS = 20;

static const char *buttonTexts[3][6] = {
    {"B1", "B2", "B3", "B4", "B5", "B6"},
    {"B1", "B2", "B3", "B4", "B5", "B6"},
    {"B1", "B2", "B3", "B4", "B5", "B6"},
};

static bool is_remote_active[3][6] = {
    {false, false, false, false, false, false},
    {false, false, false, false, false, false},
    {false, false, false, false, false, false},
};

static bool is_ui_pressed[6] = {false, false, false, false, false, false};

static String customButtonLabels[3][6];

static bool map_ui_button_to_btn_instance(uint8_t uiButton, uint16_t &instance) {
    if (uiButton < 1 || uiButton > UI_BUTTON_COUNT) {
        return false;
    }
    instance = (uint16_t)(BTN_INSTANCE_BASE + (uiButton - 1));
    return true;
}

static bool map_led_instance_to_ui_button(uint16_t instance, uint8_t &uiButton) {
    if (instance < LED_INSTANCE_BASE || instance >= (uint16_t)(LED_INSTANCE_BASE + UI_BUTTON_COUNT)) {
        return false;
    }
    uiButton = (uint8_t)(instance - LED_INSTANCE_BASE + 1);
    return true;
}

static bool map_btn_instance_to_ui_button(uint16_t instance, uint8_t &uiButton) {
    if (instance < BTN_INSTANCE_BASE || instance >= (uint16_t)(BTN_INSTANCE_BASE + UI_BUTTON_COUNT)) {
        return false;
    }
    uiButton = (uint8_t)(instance - BTN_INSTANCE_BASE + 1);
    return true;
}

static uint32_t get_selected_device_id(void) {
    uint16_t sel = lv_dropdown_get_selected(objects.drpdown_1);
    if (sel > 2) {
        sel = 0;
    }
    return DEVICE_IDS[sel];
}

static uint16_t get_selected_level(void) {
    uint16_t level = lv_dropdown_get_selected(objects.drpdown_1);
    if (level > 2) {
        level = 0;
    }
    return level;
}

static void set_status_text(const char *text) {
    lv_label_set_text(objects.lbl_1, text);
}

static void set_button_color_pressed(lv_obj_t *btn, bool isRemote) {
    uint32_t bg_color = isRemote ? COLOR_BTN_PRESSED_REM : COLOR_BTN_PRESSED_LOC;
    lv_obj_set_style_bg_color(btn, lv_color_hex(bg_color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, lv_color_hex(COLOR_BORDER_PRESSED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void set_button_color_released(lv_obj_t *btn) {
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_BTN_DEFAULT), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, lv_color_hex(COLOR_BORDER_DEFAULT), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void service_update_btn_main_visibility(void) {
    const char *txt = lv_textarea_get_text(objects.txt_1);
    uint32_t val = 0;
    if (txt != nullptr && strlen(txt) > 0) {
        val = (uint32_t)atol(txt);
    }

    if (val == 0) {
        lv_obj_add_flag(objects.btn_main, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(objects.btn_main, LV_OBJ_FLAG_HIDDEN);
    }
}

static void service_apply_device_id(void) {
    const char *txt = lv_textarea_get_text(objects.txt_1);
    if (txt == nullptr || strlen(txt) == 0) {
        return;
    }

    uint32_t newId = (uint32_t)atol(txt);
    if (newId == 0) {
        Serial.println("[SERVICE] DeviceID 0 nicht erlaubt, wird ignoriert.");
        return;
    }

    uint16_t idx = get_selected_level();
    DEVICE_IDS[idx] = newId;

    prefs.begin("hausbus", false);
    prefs.putUInt(NVS_KEY_ID[idx], DEVICE_IDS[idx]);
    prefs.end();

    Serial.printf("[SERVICE] DEVICE_IDS[%u] gespeichert: %lu\n", idx, (unsigned long)DEVICE_IDS[idx]);
    String msg = "DeviceID[" + String(idx) + "] = " + String(newId);
    set_status_text(msg.c_str());
}

static bool is_ping_request_for_selected_device(const HausbusTelegram &telegram) {
    uint32_t selectedDeviceId = get_selected_device_id();
    return telegram.valid && telegram.deviceId == selectedDeviceId && telegram.function == PING_FUNCTION &&
           telegram.instanceId == PING_INSTANCE_ID && telegram.action == PING_GET_ACTION;
}

static bool send_ping_reply_for_selected_device(const HausbusTelegram &telegram) {
    uint32_t deviceId = get_selected_device_id();
    uint16_t replyInstanceId = PING_REPLY_USE_REQUEST_INSTANCE ? telegram.instanceId : PING_INSTANCE_ID;

    String payload;
    payload.reserve(40);
    payload += String(deviceId) + "." + PING_FUNCTION + "." + String(replyInstanceId) + "." + PING_REPLY_ACTION + "." + PING_REPLY_VALUE;

    return hausbus_send_raw(payload);
}

static void update_button_labels(void) {
    uint16_t level = get_selected_level();

    lv_obj_t *labelObjs[6] = {objects.lbl_btn_1, objects.lbl_btn_2, objects.lbl_btn_3, objects.lbl_btn_4, objects.lbl_btn_5,
                              objects.lbl_btn_6};

    for (int i = 0; i < 6; i++) {
        if (labelObjs[i] == nullptr) {
            continue;
        }
        if (customButtonLabels[level][i] != "") {
            lv_label_set_text(labelObjs[i], customButtonLabels[level][i].c_str());
        } else {
            lv_label_set_text(labelObjs[i], buttonTexts[level][i]);
        }
    }
}

static void send_button_command(uint8_t buttonInstance, uint8_t value) {
    uint32_t deviceId = get_selected_device_id();
    uint16_t busButtonInstance = 0;
    if (!map_ui_button_to_btn_instance(buttonInstance, busButtonInstance)) {
        set_status_text("Ungueltige Button-Instanz");
        return;
    }

    bool ok = hausbus_send_button(deviceId, busButtonInstance, value);

    String statusText;
    statusText.reserve(64);
    statusText = (ok) ? "Gesendet: " : "Bus busy: ";
    statusText += String(deviceId);
    statusText += ".BTN.";
    statusText += String(busButtonInstance);
    statusText += ".STATUS.";
    statusText += String(value);

    set_status_text(statusText.c_str());
}

static void handle_led_telegram(const HausbusTelegram &telegram) {
    if (telegram.function != "LED") {
        return;
    }

    uint16_t instance = telegram.instanceId;
    uint8_t uiButtonInstance = 0;
    bool value = (telegram.value == "1");

    if (!map_led_instance_to_ui_button(instance, uiButtonInstance)) {
        return;
    }

    int targetLevel = -1;
    if (telegram.deviceId == DEVICE_IDS[0]) {
        targetLevel = 0;
    } else if (telegram.deviceId == DEVICE_IDS[1]) {
        targetLevel = 1;
    } else if (telegram.deviceId == DEVICE_IDS[2]) {
        targetLevel = 2;
    }

    if (targetLevel == -1) {
        return;
    }

    is_remote_active[targetLevel][uiButtonInstance - 1] = value;

    if (targetLevel == get_selected_level()) {
        lv_obj_t *targetBtn = nullptr;
        if (uiButtonInstance == 1) targetBtn = objects.btn_1;
        else if (uiButtonInstance == 2) targetBtn = objects.btn_2;
        else if (uiButtonInstance == 3) targetBtn = objects.btn_3;
        else if (uiButtonInstance == 4) targetBtn = objects.btn_4;
        else if (uiButtonInstance == 5) targetBtn = objects.btn_5;
        else if (uiButtonInstance == 6) targetBtn = objects.btn_6;

        if (targetBtn != nullptr) {
            if (value) {
                set_button_color_pressed(targetBtn, true);
            } else {
                set_button_color_released(targetBtn);
            }
        }
    }
}

static void handle_config_telegram(const HausbusTelegram &telegram) {
    if (telegram.function == "BTN") {
        if (telegram.action == "SET_CFG" && telegram.value != "") {
            uint16_t instance = telegram.instanceId;
            uint8_t uiButtonInstance = 0;
            if (map_btn_instance_to_ui_button(instance, uiButtonInstance)) {
                String newLabel = telegram.value;
                newLabel.replace("_", " ");
                if (newLabel.length() > 18) newLabel = newLabel.substring(0, 18);

                int targetLevel = -1;
                if (telegram.deviceId == DEVICE_IDS[0]) targetLevel = 0;
                else if (telegram.deviceId == DEVICE_IDS[1]) targetLevel = 1;
                else if (telegram.deviceId == DEVICE_IDS[2]) targetLevel = 2;

                if (targetLevel != -1) {
                    customButtonLabels[targetLevel][uiButtonInstance - 1] = newLabel;
                    update_button_labels();
                    set_status_text((String("Button ") + String(instance) + " Label gesetzt!").c_str());
                } else {
                    uint16_t currentLevel = get_selected_level();
                    customButtonLabels[currentLevel][uiButtonInstance - 1] = newLabel;
                    update_button_labels();
                    set_status_text("Label global gesetzt!");
                }
            }
        }
    } else if (telegram.function == "SYS" && telegram.instanceId == 1 && telegram.action == "SET_CFG") {
        if (telegram.value != "") {
            String newOptionText = telegram.value;
            newOptionText.replace("_", " ");
            if (newOptionText.length() > 20) newOptionText = newOptionText.substring(0, 20);

            int targetLevel = -1;
            if (telegram.deviceId == DEVICE_IDS[0]) targetLevel = 0;
            else if (telegram.deviceId == DEVICE_IDS[1]) targetLevel = 1;
            else if (telegram.deviceId == DEVICE_IDS[2]) targetLevel = 2;

            if (targetLevel != -1) {
                dropdown_options[targetLevel] = newOptionText;

                lv_dropdown_set_options(objects.drpdown_1, "");
                lv_dropdown_add_option(objects.drpdown_1, dropdown_options[0].c_str(), 0);
                lv_dropdown_add_option(objects.drpdown_1, dropdown_options[1].c_str(), 1);
                lv_dropdown_add_option(objects.drpdown_1, dropdown_options[2].c_str(), 2);
                lv_dropdown_set_selected(objects.drpdown_1, 0);

                set_status_text((String("Dropdown Ebene ") + String(targetLevel) + " Name geaendert!").c_str());
            }
        }
    }
}

static void handle_system_reset(const HausbusTelegram &telegram) {
    if (telegram.function != "SYS" || telegram.instanceId != 1 || telegram.action != "RESET") {
        return;
    }

    int targetLevel = -1;
    if (telegram.deviceId == DEVICE_IDS[0]) targetLevel = 0;
    else if (telegram.deviceId == DEVICE_IDS[1]) targetLevel = 1;
    else if (telegram.deviceId == DEVICE_IDS[2]) targetLevel = 2;

    if (targetLevel != -1) {
        for (int i = 0; i < 6; i++) {
            customButtonLabels[targetLevel][i] = "";
        }
        update_button_labels();
        String msg = "Reset fuer Ebene " + String(targetLevel) + " erfolgreich!";
        set_status_text(msg.c_str());
    } else {
        for (int l = 0; l < 3; l++) {
            for (int i = 0; i < 6; i++) {
                customButtonLabels[l][i] = "";
            }
        }
        update_button_labels();
        set_status_text("Globaler Reset erfolgreich!");
    }
}

static void process_incoming_telegram(void) {
    HausbusTelegram telegram;
    if (!hausbus_get_last_telegram(telegram)) {
        return;
    }

    handle_system_reset(telegram);
    handle_led_telegram(telegram);
    handle_config_telegram(telegram);

    if (is_ping_request_for_selected_device(telegram)) {
        uint16_t replyInstanceId = PING_REPLY_USE_REQUEST_INSTANCE ? telegram.instanceId : PING_INSTANCE_ID;

        String replyPayload;
        replyPayload.reserve(40);
        replyPayload += String(get_selected_device_id()) + "." + PING_FUNCTION + "." + String(replyInstanceId) + "." + PING_REPLY_ACTION + "." + PING_REPLY_VALUE;

        delay(PING_REPLY_DELAY_MS);
        bool ok = send_ping_reply_for_selected_device(telegram);

        String statusText = (ok) ? "Antwort gesendet: " : "Antwort fehlgeschlagen: ";
        statusText += replyPayload;
        set_status_text(statusText.c_str());
        return;
    }

    String rxStatus = "RX: " + telegram.rawPayload;
    set_status_text(rxStatus.c_str());
}

static void handle_button_event(lv_event_t *e, lv_obj_t *btn, uint8_t buttonInstance) {
    lv_event_code_t code = lv_event_get_code(e);
    uint8_t buttonIndex = (uint8_t)(buttonInstance - 1);
    if (buttonIndex >= UI_BUTTON_COUNT) {
        return;
    }

    if (code == LV_EVENT_PRESSED) {
        if (!is_ui_pressed[buttonIndex]) {
            is_ui_pressed[buttonIndex] = true;
            send_button_command(buttonInstance, 1);
        }

        if (!is_remote_active[get_selected_level()][buttonInstance - 1]) {
            set_button_color_pressed(btn, false);
        } else {
            set_button_color_pressed(btn, true);
        }
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (is_ui_pressed[buttonIndex]) {
            is_ui_pressed[buttonIndex] = false;
            send_button_command(buttonInstance, 0);
        }

        if (!is_remote_active[get_selected_level()][buttonInstance - 1]) {
            set_button_color_released(btn);
        }
    }
}

static void button_1_event_cb(lv_event_t *e) { handle_button_event(e, objects.btn_1, 1); }
static void button_2_event_cb(lv_event_t *e) { handle_button_event(e, objects.btn_2, 2); }
static void button_3_event_cb(lv_event_t *e) { handle_button_event(e, objects.btn_3, 3); }
static void button_4_event_cb(lv_event_t *e) { handle_button_event(e, objects.btn_4, 4); }
static void button_5_event_cb(lv_event_t *e) { handle_button_event(e, objects.btn_5, 5); }
static void button_6_event_cb(lv_event_t *e) { handle_button_event(e, objects.btn_6, 6); }

static void dropdown_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) {
        return;
    }

    update_button_labels();

    uint16_t level = get_selected_level();
    lv_obj_t *btns[6] = {objects.btn_1, objects.btn_2, objects.btn_3, objects.btn_4, objects.btn_5, objects.btn_6};

    for (int i = 0; i < 6; i++) {
        if (btns[i] != nullptr) {
            bool active = is_remote_active[level][i];
            if (active) {
                set_button_color_pressed(btns[i], true);
            } else {
                set_button_color_released(btns[i]);
            }
        }
    }

    String statusText = "Aktive DeviceID: " + String(get_selected_device_id());
    set_status_text(statusText.c_str());
}

static void btn_service_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    uint16_t idx = get_selected_level();
    String currentId = String(DEVICE_IDS[idx]);
    lv_textarea_set_text(objects.txt_1, currentId.c_str());

    service_update_btn_main_visibility();
    lv_keyboard_set_textarea(objects.key_1, objects.txt_1);
    loadScreen(SCREEN_ID_SERVICE);
}

static void txt_1_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        service_update_btn_main_visibility();
    }
}

static void key_1_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_READY) {
        service_apply_device_id();
        service_update_btn_main_visibility();
    }
}

static void btn_main_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    service_apply_device_id();
    lv_keyboard_set_textarea(objects.key_1, nullptr);
    loadScreen(SCREEN_ID_MAIN);

    uint16_t idx = get_selected_level();
    String msg = "Aktive DeviceID[" + String(idx) + "]: " + String(DEVICE_IDS[idx]);
    set_status_text(msg.c_str());
}

static void init_button_style(lv_obj_t *btn) {
    set_button_color_released(btn);
}

static void nvs_load_device_ids(void) {
    prefs.begin("hausbus", false);
    for (int i = 0; i < 3; i++) {
        DEVICE_IDS[i] = prefs.getUInt(NVS_KEY_ID[i], DEVICE_IDS_DEFAULT[i]);
        Serial.printf("[NVS] DEVICE_IDS[%d] geladen: %lu\n", i, (unsigned long)DEVICE_IDS[i]);
    }
    prefs.end();
}

void hausbus_app_begin(void) {
    nvs_load_device_ids();
    hausbus_begin(Serial1, RS485_BAUD, RS485_RX, RS485_TX);

    lvgl_port_lock(-1);

    init_button_style(objects.btn_1);
    init_button_style(objects.btn_2);
    init_button_style(objects.btn_3);
    init_button_style(objects.btn_4);
    init_button_style(objects.btn_5);
    init_button_style(objects.btn_6);

    update_button_labels();
    set_status_text(("Aktive DeviceID: " + String(DEVICE_IDS[0])).c_str());

    lv_keyboard_set_textarea(objects.key_1, objects.txt_1);
    lv_obj_add_flag(objects.btn_main, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_event_cb(objects.btn_1, button_1_event_cb, LV_EVENT_ALL, nullptr);
    lv_obj_add_event_cb(objects.btn_2, button_2_event_cb, LV_EVENT_ALL, nullptr);
    lv_obj_add_event_cb(objects.btn_3, button_3_event_cb, LV_EVENT_ALL, nullptr);
    lv_obj_add_event_cb(objects.btn_4, button_4_event_cb, LV_EVENT_ALL, nullptr);
    lv_obj_add_event_cb(objects.btn_5, button_5_event_cb, LV_EVENT_ALL, nullptr);
    lv_obj_add_event_cb(objects.btn_6, button_6_event_cb, LV_EVENT_ALL, nullptr);
    lv_obj_add_event_cb(objects.drpdown_1, dropdown_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(objects.btn_service, btn_service_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(objects.txt_1, txt_1_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(objects.key_1, key_1_event_cb, LV_EVENT_ALL, nullptr);
    lv_obj_add_event_cb(objects.btn_main, btn_main_event_cb, LV_EVENT_CLICKED, nullptr);

    lvgl_port_unlock();
}

void hausbus_app_loop(void) {
    hausbus_poll();

    if (hausbus_available()) {
        lvgl_port_lock(-1);
        process_incoming_telegram();
        lvgl_port_unlock();
    }
}