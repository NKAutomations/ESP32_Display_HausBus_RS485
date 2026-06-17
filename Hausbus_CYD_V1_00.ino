#include "touchscreen.h"
#include "ui.h"
#include "Hausbus_kernal.h"

// --------------------------------------------------
// RS485 UART Pins
// WICHTIG:
// Diese beiden Werte ggf. passend zu deinem Board setzen.
// Referenzprojekt: RX=22, TX=21
// --------------------------------------------------
static const int RS485_RX = 27;
static const int RS485_TX = 22;
static const uint32_t RS485_BAUD = 57600;

// --------------------------------------------------
// Dropdown-Ebenen / DeviceIDs
// 0 = Grundfunktionen -> 9061
// 1 = EG              -> 9062
// 2 = OG              -> 9063
// --------------------------------------------------

static const char *buttonTexts[3][6] = {
    { "Alles Ein", "Alles Aus", "Test", "Szene 1", "Szene 2", "Stop" },
    { "Kueche", "Wohnen", "Essen", "Flur", "Terrasse", "Aus" },
    { "Schlafen", "Bad", "Kind 1", "Kind 2", "Flur OG", "Aus" }
};

static uint32_t get_selected_device_id(void) {
    uint16_t sel = lv_dropdown_get_selected(objects.drpdown_1);

    switch (sel) {
        case 0: return 9061;
        case 1: return 9062;
        case 2: return 9063;
        default: return 9061;
    }
}

static uint16_t get_selected_level(void) {
    return lv_dropdown_get_selected(objects.drpdown_1);
}

static void set_status_text(const char *text) {
    lv_label_set_text(objects.lbl_2, text);
}

static void set_button_color_pressed(lv_obj_t *btn) {
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x00C853), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x009624), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void set_button_color_released(lv_obj_t *btn) {
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xA021F3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x6A00B8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void update_button_labels(void) {
    uint16_t level = get_selected_level();

    if (level > 2) {
        level = 0;
    }

    lv_label_set_text(objects.lbl_btn_1, buttonTexts[level][0]);
    lv_label_set_text(objects.lbl_btn_2, buttonTexts[level][1]);
    lv_label_set_text(objects.lbl_btn_3, buttonTexts[level][2]);
    lv_label_set_text(objects.lbl_btn_4, buttonTexts[level][3]);
    lv_label_set_text(objects.lbl_btn_5, buttonTexts[level][4]);
    lv_label_set_text(objects.lbl_btn_6, buttonTexts[level][5]);
}

static void send_button_command(uint8_t buttonInstance, uint8_t value) {
    uint32_t deviceId = get_selected_device_id();

    bool ok = hausbus_send_button(deviceId, buttonInstance, value);

    String statusText;
    statusText.reserve(64);

    if (ok) {
        statusText = "Gesendet: ";
    } else {
        statusText = "Bus busy: ";
    }

    statusText += String(deviceId);
    statusText += ".BTN.";
    statusText += String(buttonInstance);
    statusText += ".STATUS.";
    statusText += String(value);

    set_status_text(statusText.c_str());
}

static void handle_button_event(lv_event_t *e, lv_obj_t *btn, uint8_t buttonInstance) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        set_button_color_pressed(btn);
        send_button_command(buttonInstance, 1);
    }
    else if (code == LV_EVENT_RELEASED) {
        set_button_color_released(btn);
        send_button_command(buttonInstance, 0);
    }
    else if (code == LV_EVENT_PRESS_LOST) {
        set_button_color_released(btn);
    }
}

static void button_1_event_cb(lv_event_t *e) {
    handle_button_event(e, objects.btn_1, 1);
}

static void button_2_event_cb(lv_event_t *e) {
    handle_button_event(e, objects.btn_2, 2);
}

static void button_3_event_cb(lv_event_t *e) {
    handle_button_event(e, objects.btn_3, 3);
}

static void button_4_event_cb(lv_event_t *e) {
    handle_button_event(e, objects.btn_4, 4);
}

static void button_5_event_cb(lv_event_t *e) {
    handle_button_event(e, objects.btn_5, 5);
}

static void button_6_event_cb(lv_event_t *e) {
    handle_button_event(e, objects.btn_6, 6);
}

static void dropdown_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        uint32_t deviceId = get_selected_device_id();

        update_button_labels();

        String statusText = "Aktive DeviceID: ";
        statusText += String(deviceId);
        set_status_text(statusText.c_str());
    }
}

static void init_button_style(lv_obj_t *btn) {
    set_button_color_released(btn);
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("[APP] Start");

    touchscreen_setup();
    ui_init();

    hausbus_begin(Serial2, RS485_BAUD, RS485_RX, RS485_TX);

    init_button_style(objects.btn_1);
    init_button_style(objects.btn_2);
    init_button_style(objects.btn_3);
    init_button_style(objects.btn_4);
    init_button_style(objects.btn_5);
    init_button_style(objects.btn_6);

    update_button_labels();
    set_status_text("Aktive DeviceID: 9061");

    lv_obj_add_event_cb(objects.btn_1, button_1_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.btn_2, button_2_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.btn_3, button_3_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.btn_4, button_4_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.btn_5, button_5_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.btn_6, button_6_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.drpdown_1, dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

void loop() {
    hausbus_poll();

    if (hausbus_available()) {
        String rx = hausbus_get_last_payload();
        String statusText = "RX: ";
        statusText += rx;
        set_status_text(statusText.c_str());
    }

    lv_task_handler();
    lv_tick_inc(5);
    delay(5);
}
