#include <Arduino.h>
#include <esp_display_panel.hpp>
#include "lvgl_v8_port.h"
#include "ui/ui.h"
#include "hausbus_app.h"

using namespace esp_panel::board;

static Board *panel = nullptr;

void setup() {
    Serial.begin(115200);

    panel = new Board();
    panel->init();
    panel->begin();

    lvgl_port_init(panel->getLCD(), panel->getTouch());

    lvgl_port_lock(-1);
    ui_init();
    lvgl_port_unlock();

    hausbus_app_begin();
}

void loop() {
    hausbus_app_loop();
    delay(5);
}
