#ifndef HAUSBUS_KERNAL_H
#define HAUSBUS_KERNAL_H

#include <Arduino.h>

struct HausbusTelegram {
    String rawPayload;
    uint32_t deviceId;
    String function;
    uint16_t instanceId;
    String action;
    String value;
    bool valid;
    bool matchesActiveDevice;
};

void hausbus_begin(HardwareSerial &serialPort, uint32_t baud, int rxPin, int txPin);
void hausbus_poll(void);

bool hausbus_send_raw(const String &payload);
bool hausbus_send_button(uint32_t deviceId, uint16_t buttonInstance, uint8_t value);

bool hausbus_available(void);
String hausbus_get_last_payload(void);
bool hausbus_get_last_telegram(HausbusTelegram &telegram);

#endif