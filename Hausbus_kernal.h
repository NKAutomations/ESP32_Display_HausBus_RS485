#ifndef HAUSBUS_KERNAL_H
#define HAUSBUS_KERNAL_H

#include <Arduino.h>

void hausbus_begin(HardwareSerial &serialPort, uint32_t baud, int rxPin, int txPin);
void hausbus_poll(void);

bool hausbus_send_raw(const String &payload);
bool hausbus_send_button(uint32_t deviceId, uint16_t buttonInstance, uint8_t value);

bool hausbus_available(void);
String hausbus_get_last_payload(void);

#endif
