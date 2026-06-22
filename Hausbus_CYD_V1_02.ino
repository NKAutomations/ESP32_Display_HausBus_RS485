// ============================================================
// Hausbus_CYD_V1_02.ino
// Basierend auf V1_01 + Service-Seite mit DeviceID-Aenderung
// + Persistenz der DeviceIDs via ESP32 NVS (Preferences)
// ============================================================

#include "touchscreen.h"
#include "ui.h"
#include "Hausbus_kernal.h"
#include <Preferences.h>

// NVS-Instanz (Namespace "hausbus")
static Preferences prefs;

// --------------------------------------------------
// RS485 UART Pins
// --------------------------------------------------
static const int RS485_RX = 35;
static const int RS485_TX = 22;
static const uint32_t RS485_BAUD = 57600;

// --------------------------------------------------
// Dropdown-Ebenen / DeviceIDs
// 0 = Grundfunktionen -> 9061
// 1 = EG              -> 9062
// 2 = OG              -> 9063
// --------------------------------------------------
String dropdown_options[3] = {"Ebene1", "Ebene2", "OEbene3"};

// Standard-Fallback-IDs (werden beim ersten Start in NVS geschrieben)
static const uint32_t DEVICE_IDS_DEFAULT[3] = {9061, 9062, 9063};

// Laufzeit-Array – wird aus NVS geladen, kann per Service-Seite geaendert werden
uint32_t DEVICE_IDS[3] = {9061, 9062, 9063};

// NVS-Schluesselnamen fuer die drei DeviceIDs
static const char *NVS_KEY_ID[3] = {"devid_0", "devid_1", "devid_2"};

// Alle DeviceIDs aus NVS laden (Fallback auf Defaults)
static void nvs_load_device_ids(void) {
    prefs.begin("hausbus", /*readOnly=*/false);
    for (int i = 0; i < 3; i++) {
        DEVICE_IDS[i] = prefs.getUInt(NVS_KEY_ID[i], DEVICE_IDS_DEFAULT[i]);
        Serial.printf("[NVS] DEVICE_IDS[%d] geladen: %lu\n", i, (unsigned long)DEVICE_IDS[i]);
    }
    prefs.end();
}

// Einzelne DeviceID in NVS speichern
static void nvs_save_device_id(uint16_t idx) {
    if (idx > 2) return;
    prefs.begin("hausbus", /*readOnly=*/false);
    prefs.putUInt(NVS_KEY_ID[idx], DEVICE_IDS[idx]);
    prefs.end();
    Serial.printf("[NVS] DEVICE_IDS[%d] gespeichert: %lu\n", (int)idx, (unsigned long)DEVICE_IDS[idx]);
}

// --- Farbdefinitionen ---
static const uint32_t COLOR_BTN_DEFAULT      = 0xA021F3; // Lila (Standard/Released)
static const uint32_t COLOR_BTN_PRESSED_LOC  = 0x00C853; // Gruen (Lokal gedrueckt)
static const uint32_t COLOR_BTN_PRESSED_REM  = 0xFFA500; // Orange (Remote Befehl)

// Rahmenfarben
static const uint32_t COLOR_BORDER_DEFAULT   = 0x6A00B8; // Lila Rahmen
static const uint32_t COLOR_BORDER_PRESSED   = 0x009624; // Gruener Rahmen

// --------------------------------------------------
// Ping / gSTATUS / rSTATUS Definition
// --------------------------------------------------
static const char    *PING_FUNCTION                    = "OUT";
static const uint16_t PING_INSTANCE_ID                 = 210;
static const bool     PING_REPLY_USE_REQUEST_INSTANCE  = true;
static const char    *PING_GET_ACTION                  = "gSTATUS";
static const char    *PING_REPLY_ACTION                = "rSTATUS";
static const char    *PING_REPLY_VALUE                 = "1";
static const uint32_t PING_REPLY_DELAY_MS              = 20;

// Standard-Texte fuer Buttons
static const char *buttonTexts[3][6] = {
    { "B1", "B2", "B3", "B4", "B5", "B6" },
    { "B1", "B2", "B3", "B4", "B5", "B6" },
    { "B1", "B2", "B3", "B4", "B5", "B6" },
};

// Speichert, ob ein Button remote aktiv ist (Index 0 = Button 1 usw.)
static bool is_remote_active[6] = {false, false, false, false, false, false};

// Speicher fuer benutzerdefinierte Button-Namen (RAM)
String customButtonLabels[3][6];

// ===========================================================
// Hilfsfunktionen
// ===========================================================

static uint32_t get_selected_device_id(void) {
    uint16_t sel = lv_dropdown_get_selected(objects.drpdown_1);
    if (sel > 2) sel = 0;
    return DEVICE_IDS[sel];
}

static uint16_t get_selected_level(void) {
    uint16_t level = lv_dropdown_get_selected(objects.drpdown_1);
    if (level > 2) level = 0;
    return level;
}

static void set_status_text(const char *text) {
    lv_label_set_text(objects.lbl_1, text);
}

static void set_button_color_pressed(lv_obj_t *btn, bool isRemote) {
    uint32_t bg_color     = isRemote ? COLOR_BTN_PRESSED_REM : COLOR_BTN_PRESSED_LOC;
    uint32_t border_color = COLOR_BORDER_PRESSED;
    lv_obj_set_style_bg_color(btn, lv_color_hex(bg_color),     LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, lv_color_hex(border_color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn, 2,                      LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void set_button_color_released(lv_obj_t *btn) {
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_BTN_DEFAULT),    LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, lv_color_hex(COLOR_BORDER_DEFAULT), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn, 2,                              LV_PART_MAIN | LV_STATE_DEFAULT);
}

// ===========================================================
// Service-Seite: DeviceID-Verwaltung
// ===========================================================

// Prueft den aktuellen Textarea-Inhalt und blendet btn_main
// aus, wenn der Wert 0 oder leer ist.
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

// Wird aufgerufen, wenn der Benutzer auf der Service-Seite
// die Taste "Ok" (Enter/Bestaetigen) auf der Tastatur drueckt
// oder wenn er btn_main bestaetigt -> uebernimmt die DeviceID.
static void service_apply_device_id(void) {
    const char *txt = lv_textarea_get_text(objects.txt_1);
    if (txt == nullptr || strlen(txt) == 0) return;

    uint32_t newId = (uint32_t)atol(txt);
    if (newId == 0) {
        Serial.println("[SERVICE] DeviceID 0 nicht erlaubt, wird ignoriert.");
        return;
    }

    uint16_t idx = get_selected_level();
    DEVICE_IDS[idx] = newId;

    // In NVS (Flash) dauerhaft speichern
    nvs_save_device_id(idx);

    Serial.print("[SERVICE] DeviceID fuer Index ");
    Serial.print(idx);
    Serial.print(" geaendert auf: ");
    Serial.println(newId);

    // Statusanzeige auf Main-Seite aktualisieren (wird nach Rueckkehr sichtbar)
    String msg = "DeviceID[" + String(idx) + "] = " + String(newId);
    set_status_text(msg.c_str());
}

// ===========================================================
// LED-Telegramm-Handler
// ===========================================================

static void handle_led_telegram(const HausbusTelegram &telegram) {
    if (telegram.function == "LED" && telegram.action == "ON") {
        uint16_t instance = telegram.instanceId;
        bool value = (telegram.value == "1");

        if (instance >= 1 && instance <= 6) {
            lv_obj_t *targetBtn = nullptr;
            if      (instance == 1) targetBtn = objects.btn_1;
            else if (instance == 2) targetBtn = objects.btn_2;
            else if (instance == 3) targetBtn = objects.btn_3;
            else if (instance == 4) targetBtn = objects.btn_4;
            else if (instance == 5) targetBtn = objects.btn_5;
            else if (instance == 6) targetBtn = objects.btn_6;

            if (targetBtn != nullptr) {
                is_remote_active[instance - 1] = value;
                if (value) {
                    set_button_color_pressed(targetBtn, true);
                } else {
                    set_button_color_released(targetBtn);
                }
            }
        }
    }
}

// ===========================================================
// UI Update Logik
// ===========================================================

static void update_button_labels(void) {
    uint16_t level = get_selected_level();

    lv_obj_t *labelObjs[6] = {
        objects.lbl_btn_1,
        objects.lbl_btn_2,
        objects.lbl_btn_3,
        objects.lbl_btn_4,
        objects.lbl_btn_5,
        objects.lbl_btn_6
    };

    for (int i = 0; i < 6; i++) {
        if (labelObjs[i] == nullptr) continue;
        if (customButtonLabels[level][i] != "") {
            lv_label_set_text(labelObjs[i], customButtonLabels[level][i].c_str());
        } else {
            lv_label_set_text(labelObjs[i], buttonTexts[level][i]);
        }
    }
}

// ===========================================================
// Hausbus Kommunikation
// ===========================================================

static void send_button_command(uint8_t buttonInstance, uint8_t value) {
    uint32_t deviceId = get_selected_device_id();
    bool ok = hausbus_send_button(deviceId, buttonInstance, value);

    String statusText;
    statusText.reserve(64);
    statusText  = (ok) ? "Gesendet: " : "Bus busy: ";
    statusText += String(deviceId);
    statusText += ".BTN.";
    statusText += String(buttonInstance);
    statusText += ".STATUS.";
    statusText += String(value);

    set_status_text(statusText.c_str());
}

static bool is_ping_request_for_selected_device(const HausbusTelegram &telegram) {
    uint32_t selectedDeviceId = get_selected_device_id();
    if (!telegram.valid
        || telegram.deviceId    != selectedDeviceId
        || telegram.function    != PING_FUNCTION
        || telegram.instanceId  != PING_INSTANCE_ID
        || telegram.action      != PING_GET_ACTION) {
        return false;
    }
    Serial.print("[PING] Match fuer Device ");
    Serial.print(selectedDeviceId);
    Serial.print(" Inst ");
    Serial.println(telegram.instanceId);
    return true;
}

static bool send_ping_reply_for_selected_device(const HausbusTelegram &telegram) {
    uint32_t deviceId       = get_selected_device_id();
    uint16_t replyInstanceId = PING_REPLY_USE_REQUEST_INSTANCE ? telegram.instanceId : PING_INSTANCE_ID;

    String payload;
    payload.reserve(40);
    payload += String(deviceId) + "." + PING_FUNCTION + "." +
               String(replyInstanceId) + "." + PING_REPLY_ACTION + "." + PING_REPLY_VALUE;

    return hausbus_send_raw(payload);
}

static void handle_config_telegram(const HausbusTelegram &telegram) {
    // FALL A: Button-Konfiguration (BTN.x.SET_CFG.Text)
    if (telegram.function == "BTN") {
        if (telegram.action == "SET_CFG" && telegram.value != "") {
            uint16_t instance = telegram.instanceId;
            if (instance >= 1 && instance <= 6) {
                String newLabel = telegram.value;
                newLabel.replace("_", " ");
                if (newLabel.length() > 18) newLabel = newLabel.substring(0, 18);

                int targetLevel = -1;
                if      (telegram.deviceId == DEVICE_IDS[0]) targetLevel = 0;
                else if (telegram.deviceId == DEVICE_IDS[1]) targetLevel = 1;
                else if (telegram.deviceId == DEVICE_IDS[2]) targetLevel = 2;

                if (targetLevel != -1) {
                    customButtonLabels[targetLevel][instance - 1] = newLabel;
                    update_button_labels();
                    set_status_text((String("Button ") + String(instance) + " Label gesetzt!").c_str());
                } else {
                    uint16_t currentLevel = get_selected_level();
                    customButtonLabels[currentLevel][instance - 1] = newLabel;
                    update_button_labels();
                    set_status_text("Label global gesetzt!");
                }
            }
        }
    }
    // FALL B: Dropdown-Konfiguration (SYS.1.SET_CFG.Text)
    else if (telegram.function == "SYS" && telegram.instanceId == 1 && telegram.action == "SET_CFG") {
        if (telegram.value != "") {
            String newOptionText = telegram.value;
            newOptionText.replace("_", " ");
            if (newOptionText.length() > 20) newOptionText = newOptionText.substring(0, 20);

            int targetLevel = -1;
            if      (telegram.deviceId == DEVICE_IDS[0]) targetLevel = 0;
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
    if (telegram.function == "SYS" && telegram.instanceId == 1 && telegram.action == "RESET") {
        int targetLevel = -1;
        if      (telegram.deviceId == DEVICE_IDS[0]) targetLevel = 0;
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
}

static void process_incoming_telegram(void) {
    HausbusTelegram telegram;
    if (!hausbus_get_last_telegram(telegram)) return;

    Serial.println("[DEBUG] --- Telegramm empfangen ---");
    Serial.print("[DEBUG] RAW: "); Serial.println(telegram.rawPayload);

    handle_system_reset(telegram);
    handle_led_telegram(telegram);
    handle_config_telegram(telegram);

    if (is_ping_request_for_selected_device(telegram)) {
        uint16_t replyInstanceId = PING_REPLY_USE_REQUEST_INSTANCE ? telegram.instanceId : PING_INSTANCE_ID;

        String replyPayload;
        replyPayload.reserve(40);
        replyPayload += String(get_selected_device_id()) + "." + PING_FUNCTION + "." +
                        String(replyInstanceId) + "." + PING_REPLY_ACTION + "." + PING_REPLY_VALUE;

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

// ===========================================================
// Event Handler
// ===========================================================

static void handle_button_event(lv_event_t *e, lv_obj_t *btn, uint8_t buttonInstance) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        if (!is_remote_active[buttonInstance - 1]) {
            set_button_color_pressed(btn, false);
        } else {
            set_button_color_pressed(btn, true);
        }
        send_button_command(buttonInstance, 1);
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (!is_remote_active[buttonInstance - 1]) {
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
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        update_button_labels();
        String statusText = "Aktive DeviceID: " + String(get_selected_device_id());
        set_status_text(statusText.c_str());
    }
}

// --- Service-Seite: btn_service (Main -> Service) ---
static void btn_service_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        // Textarea mit aktueller DeviceID des gewaelten Index befuellen
        uint16_t idx = get_selected_level();
        String currentId = String(DEVICE_IDS[idx]);
        lv_textarea_set_text(objects.txt_1, currentId.c_str());

        // Sichtbarkeit von btn_main sicherstellen
        service_update_btn_main_visibility();

        // Keyboard mit Textarea verknuepfen
        lv_keyboard_set_textarea(objects.key_1, objects.txt_1);

        loadScreen(SCREEN_ID_SERVICE);
    }
}

// --- Service-Seite: Textarea-Event (Eingabe aendert sich) ---
static void txt_1_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        // btn_main ein-/ausblenden je nach Inhalt
        service_update_btn_main_visibility();
    }
}

// --- Service-Seite: Keyboard-Event ---
// Verarbeitet "Ok"-Taste der Tastatur (LV_EVENT_READY = Enter/Ok auf LVGL-Keyboard)
static void key_1_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    // LV_EVENT_READY wird ausgeloest, wenn der User die Enter/Ok-Taste drueckt
    if (code == LV_EVENT_READY) {
        // DeviceID uebernehmen und validieren
        service_apply_device_id();
        // Sichtbarkeit pruefen und ggf. blenden
        service_update_btn_main_visibility();
    }
    // LV_EVENT_CANCEL: optionale Cancel-Taste - einfach ignorieren oder zum Main zurueck
    // if (code == LV_EVENT_CANCEL) { loadScreen(SCREEN_ID_MAIN); }
}

// --- Service-Seite: btn_main (Service -> Main) ---
static void btn_main_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        // DeviceID sichern bevor wir wechseln
        service_apply_device_id();

        // Keyboard-Verknuepfung aufheben
        lv_keyboard_set_textarea(objects.key_1, nullptr);

        loadScreen(SCREEN_ID_MAIN);

        // Status auf Main-Seite aktualisieren
        uint16_t idx = get_selected_level();
        String msg = "Aktive DeviceID[" + String(idx) + "]: " + String(DEVICE_IDS[idx]);
        set_status_text(msg.c_str());
    }
}

static void init_button_style(lv_obj_t *btn) {
    set_button_color_released(btn);
}

// ===========================================================
// Setup & Loop
// ===========================================================

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("[APP] Start - Hausbus CYD V1.02");

    touchscreen_setup();
    ui_init();

    // DeviceIDs aus NVS laden (persistent, ueberschreibt die Defaults)
    nvs_load_device_ids();

    hausbus_begin(Serial2, RS485_BAUD, RS485_RX, RS485_TX);

    // Button-Styles initialisieren
    init_button_style(objects.btn_1);
    init_button_style(objects.btn_2);
    init_button_style(objects.btn_3);
    init_button_style(objects.btn_4);
    init_button_style(objects.btn_5);
    init_button_style(objects.btn_6);

    // Button-Labels initialisieren
    update_button_labels();
    set_status_text(("Aktive DeviceID: " + String(DEVICE_IDS[0])).c_str());

    // Keyboard mit Textarea verknuepfen (Service-Seite)
    lv_keyboard_set_textarea(objects.key_1, objects.txt_1);

    // btn_main initial ausblenden (Textarea ist zu Beginn leer / 0)
    lv_obj_add_flag(objects.btn_main, LV_OBJ_FLAG_HIDDEN);

    // --- Event-Callbacks registrieren ---

    // Main-Seite: 6 Buttons
    lv_obj_add_event_cb(objects.btn_1, button_1_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.btn_2, button_2_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.btn_3, button_3_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.btn_4, button_4_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.btn_5, button_5_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.btn_6, button_6_event_cb, LV_EVENT_ALL, NULL);

    // Main-Seite: Dropdown
    lv_obj_add_event_cb(objects.drpdown_1, dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Main-Seite: Service-Button
    lv_obj_add_event_cb(objects.btn_service, btn_service_event_cb, LV_EVENT_CLICKED, NULL);

    // Service-Seite: Textarea (Eingabe-Aenderung)
    lv_obj_add_event_cb(objects.txt_1, txt_1_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Service-Seite: Keyboard (Ok-Taste)
    lv_obj_add_event_cb(objects.key_1, key_1_event_cb, LV_EVENT_ALL, NULL);

    // Service-Seite: Zurueck-Button
    lv_obj_add_event_cb(objects.btn_main, btn_main_event_cb, LV_EVENT_CLICKED, NULL);
}

void loop() {
    hausbus_poll();

    if (hausbus_available()) {
        process_incoming_telegram();
    }

    lv_task_handler();
    lv_tick_inc(5);
    delay(5);
}
