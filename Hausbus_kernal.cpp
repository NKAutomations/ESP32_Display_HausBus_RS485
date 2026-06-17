#include "Hausbus_kernal.h"

static HardwareSerial *g_busSerial = nullptr;

static int g_rxPin = -1;
static int g_txPin = -1;
static uint32_t g_baud = 57600;

static volatile uint32_t g_lastRxByteMicros = 0;

static bool g_inFrame = false;
static String g_rxBuf = "";
static String g_lastPayload = "";
static bool g_payloadAvailable = false;

static const uint8_t START_BYTE = 0xFD;
static const uint8_t END_BYTE = 0xFE;

static const uint32_t BACKOFF_MIN_MS = 2;
static const uint32_t BACKOFF_MAX_MS = 20;
static const uint32_t BUS_WAIT_MAX_MS = 250;
static const uint32_t BUS_IDLE_TIME_MS = 10;

static inline bool isBusIdleNow(void) {
    return (millis() - (g_lastRxByteMicros / 1000UL)) >= BUS_IDLE_TIME_MS;
}

static String buildTelegram(const String &payload) {
    String telegram;
    telegram.reserve(payload.length() + 2);
    telegram += (char)START_BYTE;
    telegram += payload;
    telegram += (char)END_BYTE;
    return telegram;
}

static void sendFullTelegram(const String &telegram) {
    if (g_busSerial == nullptr) {
        return;
    }

    g_busSerial->print(telegram);
    g_busSerial->flush();

    Serial.print("[TX] ");
    Serial.println(telegram);
}

static bool sendRawTelegramSoftBusFree(const String &payload, uint32_t maxWaitMs) {
    if (g_busSerial == nullptr) {
        return false;
    }

    String telegram = buildTelegram(payload);

    uint32_t start = millis();

    while ((millis() - start) < maxWaitMs) {
        hausbus_poll();
        if (isBusIdleNow()) {
            break;
        }
        delay(1);
    }

    if (!isBusIdleNow()) {
        Serial.println("[HAUSBUS] Bus busy - timeout");
        return false;
    }

    uint32_t backoff = random(BACKOFF_MIN_MS, BACKOFF_MAX_MS + 1);
    uint32_t t0 = millis();

    while ((millis() - t0) < backoff) {
        hausbus_poll();
        if (!isBusIdleNow()) {
            Serial.println("[HAUSBUS] Bus busy during backoff");
            return false;
        }
        delay(1);
    }

    if (!isBusIdleNow()) {
        Serial.println("[HAUSBUS] Bus busy before send");
        return false;
    }

    sendFullTelegram(telegram);
    return true;
}

void hausbus_begin(HardwareSerial &serialPort, uint32_t baud, int rxPin, int txPin) {
    g_busSerial = &serialPort;
    g_baud = baud;
    g_rxPin = rxPin;
    g_txPin = txPin;

    g_busSerial->begin(g_baud, SERIAL_8E1, g_rxPin, g_txPin);
    g_busSerial->setTimeout(10);

    g_lastRxByteMicros = micros();
    g_inFrame = false;
    g_rxBuf = "";
    g_lastPayload = "";
    g_payloadAvailable = false;

    randomSeed((uint32_t)esp_random());

    while (g_busSerial->available()) {
        g_busSerial->read();
    }

    Serial.println("[HAUSBUS] Init done");
    Serial.printf("[HAUSBUS] RX=%d TX=%d BAUD=%lu 8E1\n", g_rxPin, g_txPin, (unsigned long)g_baud);
}

void hausbus_poll(void) {
    if (g_busSerial == nullptr) {
        return;
    }

    while (g_busSerial->available() > 0) {
        int b = g_busSerial->read();
        if (b < 0) {
            break;
        }

        g_lastRxByteMicros = micros();

        if (!g_inFrame) {
            if ((uint8_t)b == START_BYTE) {
                g_inFrame = true;
                g_rxBuf = "";
            }
            continue;
        }

        if ((uint8_t)b == END_BYTE) {
            g_inFrame = false;
            g_lastPayload = g_rxBuf;
            g_payloadAvailable = true;

            Serial.print("[RX] ");
            Serial.println(g_lastPayload);

            g_rxBuf = "";
            continue;
        }

        if (g_rxBuf.length() < 220) {
            g_rxBuf += (char)b;
        } else {
            g_inFrame = false;
            g_rxBuf = "";
        }
    }
}

bool hausbus_send_raw(const String &payload) {
    return sendRawTelegramSoftBusFree(payload, BUS_WAIT_MAX_MS);
}

bool hausbus_send_button(uint32_t deviceId, uint16_t buttonInstance, uint8_t value) {
    String payload;
    payload.reserve(32);

    payload += String(deviceId);
    payload += ".BTN.";
    payload += String(buttonInstance);
    payload += ".STATUS.";
    payload += String(value);

    return hausbus_send_raw(payload);
}

bool hausbus_available(void) {
    return g_payloadAvailable;
}

String hausbus_get_last_payload(void) {
    g_payloadAvailable = false;
    return g_lastPayload;
}
