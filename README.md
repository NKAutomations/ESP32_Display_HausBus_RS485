# Hausbus CYD RS485 Test Project

Dieses Projekt verbindet ein **ESP32-2432S028R (Cheap Yellow Display / CYD)** mit einer **RS485-/HausBus-Kommunikation** und einer **LVGL-Oberfläche**. Ziel ist es, über sechs Touch-Buttons Telegramme an verschiedene DeviceIDs zu senden und eingehende Telegramme auf dem Display anzuzeigen.

## Projektziel

Die Anwendung dient als einfacher HMI-Testaufbau für den HausBus:

- Auswahl einer Zielgruppe per Dropdown
- Senden von Button-Status über RS485
- Empfangen und Anzeigen von HausBus-Payloads
- Trennung zwischen UI-Logik und Bus-Kommunikation

## Hardware-Hinweis

Für das hier verwendete Board `ESP32-2432S028R` wurde die funktionierende UART-Verdrahtung mit folgenden Pins getestet:

- **RS485 RX am ESP32:** `GPIO22`
- **RS485 TX am ESP32:** `GPIO27`

Im Sketch `Hausbus_CYD_V1_00.ino` sollten diese Pins entsprechend gesetzt werden.

## Dateistruktur

### `Hausbus_CYD_V1_00.ino`

Dies ist der Hauptsketch der Anwendung. Er enthält:

- Initialisierung von Display, Touch und LVGL-UI
- Start der HausBus-Kommunikation über `hausbus_begin(...)`
- Definition der RS485-Pins und Baudrate
- Dropdown-Logik zur Auswahl der DeviceID
- Zuordnung von sechs Buttons zu sechs Button-Instanzen
- Senden von `STATUS.1` bei Tastendruck und `STATUS.0` beim Loslassen
- Anzeige des letzten gesendeten oder empfangenen Telegramm-Inhalts auf dem Display

#### Zentrale Funktionen in dieser Datei

- `get_selected_device_id()`
  - ordnet den Dropdown-Einträgen die DeviceIDs `9061`, `9062` und `9063` zu
- `update_button_labels()`
  - ändert die sichtbaren Button-Texte je nach gewählter Ebene
- `send_button_command(...)`
  - baut den Status-Text für die UI auf und sendet den Button-Befehl an den HausBus
- `handle_button_event(...)`
  - verarbeitet `PRESSED`, `RELEASED` und `PRESS_LOST`
- `loop()`
  - ruft fortlaufend `hausbus_poll()` und den LVGL-Taskhandler auf

#### Gesendetes Telegrammformat

Die Buttons senden Payloads in diesem Format:

`<DeviceID>.BTN.<ButtonInstanz>.STATUS.<Wert>`

Beispiel:

`9062.BTN.3.STATUS.1`

Diese Payload wird im Kommunikationsmodul anschließend mit Start- und Endbyte eingerahmt.

---

### `Hausbus_kernal.h`

Dies ist die Header-Datei des Kommunikationsmoduls. Sie deklariert die öffentliche Schnittstelle für die HausBus-Funktionen.

#### Enthaltene Funktionen

- `hausbus_begin(HardwareSerial &serialPort, uint32_t baud, int rxPin, int txPin)`
  - initialisiert die serielle HausBus-Schnittstelle
- `hausbus_poll()`
  - liest eingehende Bytes ein und setzt empfangene Frames zusammen
- `hausbus_send_raw(const String &payload)`
  - sendet eine frei aufgebaute Payload
- `hausbus_send_button(uint32_t deviceId, uint16_t buttonInstance, uint8_t value)`
  - Komfortfunktion zum Senden eines Button-Telegramms
- `hausbus_available()`
  - meldet, ob eine neue empfangene Payload vorliegt
- `hausbus_get_last_payload()`
  - liefert die zuletzt empfangene Payload zurück

Die Datei kapselt damit die komplette öffentliche API des HausBus-Kerns.

---

### `Hausbus_kernal.cpp`

Dies ist die Implementierung des Kommunikationskerns für den HausBus. Die Datei enthält die komplette RS485-/UART-Logik.

#### Aufgaben dieser Datei

- Initialisierung von `HardwareSerial`
- Betrieb mit **57600 Baud, 8E1**
- Aufbau kompletter Telegramme mit:
  - **Startbyte:** `0xFD`
  - **Endbyte:** `0xFE`
- Einlesen eingehender Frames
- Zwischenspeichern der letzten empfangenen Payload
- einfache Bus-Frei-Erkennung mit Soft-Backoff vor dem Senden
- serielle Debug-Ausgaben über USB (`Serial`)

#### Wichtige interne Bestandteile

- `buildTelegram(...)`
  - ergänzt Start- und Endbyte um die Nutzlast
- `sendFullTelegram(...)`
  - sendet das komplette Telegramm über UART
- `sendRawTelegramSoftBusFree(...)`
  - wartet auf einen freien Bus und führt vor dem Senden ein kleines Zufalls-Backoff aus
- `hausbus_poll()`
  - erkennt Start- und Endbyte und rekonstruiert daraus die Payload

#### Kommunikationsprinzip

Die eigentliche Payload wird nicht direkt roh versendet, sondern in folgendes Frame-Schema eingebettet:

`0xFD <PAYLOAD> 0xFE`

Beispiel:

`0xFD 9061.BTN.1.STATUS.1 0xFE`

Damit ist die Trennung zwischen Nutzdaten und Telegrammrahmen klar umgesetzt.

## Bedienlogik

Die Oberfläche arbeitet mit drei auswählbaren Ebenen:

- **Grundfunktionen** -> `9061`
- **EG** -> `9062`
- **OG** -> `9063`

Je nach Auswahl ändern sich die Button-Beschriftungen. Die sechs Buttons senden jeweils ihre eigene Button-Instanz von `1` bis `6`.

## Ablauf beim Tastendruck

1. Benutzer drückt einen Button auf dem Touchscreen.
2. Der Button wird farblich als gedrückt markiert.
3. Es wird ein Telegramm mit `STATUS.1` gesendet.
4. Beim Loslassen wird die Farbe zurückgesetzt.
5. Danach wird ein Telegramm mit `STATUS.0` gesendet.

## Voraussetzungen

Für das Projekt werden unter anderem folgende Bestandteile vorausgesetzt:

- ESP32-CYD-Projektumgebung
- Arduino-Framework
- LVGL
- die projektbezogenen Dateien `touchscreen.h` und `ui.h`
- ein passendes RS485-Transceiver-Modul
- korrekte UART-Verdrahtung zwischen CYD und RS485-Modul

## Hinweise

- In `Hausbus_CYD_V1_00.ino` ist ein Kommentar enthalten, der noch auf ein Referenzprojekt mit `RX=22` und `TX=21` verweist. Für dieses konkrete Setup sollte die tatsächlich funktionierende Verdrahtung dokumentiert und im Code konsistent gehalten werden.
- Der Dateiname `Hausbus_kernal` enthält das Wort `kernal`. Falls gewünscht, kann dieser später noch in `kernel` umbenannt werden. Dann müssen allerdings auch die Includes angepasst werden.
- Die USB-Debug-Ausgaben wie `[TX]`, `[RX]` oder `[HAUSBUS]` dienen nur der Diagnose im seriellen Monitor und sind nicht Bestandteil der Nutzdaten auf dem Bus.

## Mögliche nächste Schritte

- Pinbelegung im Hauptsketch final auf den verifizierten Stand bringen
- Readme um Verdrahtungsbild ergänzen
- Beispiele für gesendete und empfangene Telegramme hinzufügen
- DeviceIDs und Button-Texte in Konfigurationsstrukturen auslagern
- Kollisionserkennung und Retry-Strategie weiter an das Referenzprojekt anlehnen

## Lizenz / Nutzung

Dieses Repository ist aktuell als Projekt- und Teststand für die Entwicklung einer CYD-basierten HausBus-Bedienoberfläche gedacht. Eine formale Lizenz kann bei Bedarf noch ergänzt werden.
