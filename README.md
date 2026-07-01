# Hausbus Display Controller (CYD + Waveshare 4.3B)

Dieses Repository enthält zwei Varianten der Hausbus-Steuerung mit LVGL-Oberfläche:

- CYD (Cheap Yellow Display)
- Waveshare ESP32-S3-Touch-LCD-4.3B

Beide Varianten nutzen das Hausbus-Protokoll über RS485/UART, inklusive Device-ID-Verwaltung, Label-Konfiguration und Status-Rückmeldungen.

## Repository-Aufbau
- CYD: Implementierung und Referenzcode im Ordner CYD
- Waveshare 4.3B PlatformIO-Projekt: Ordner Waveshare43b

## Waveshare 4.3B (PlatformIO)

Pfad: Waveshare43b

### Enthaltene Funktionen
- Vollständige Hausbus-Logik (Parser, TX/RX, Backoff, Polling)
- Service-Seite für Device-ID-Konfiguration via NVS (Preferences)
- Dynamische Label-Updates über BTN.SET_CFG
- Dropdown-Konfiguration über SYS.1.SET_CFG
- Reset-Verarbeitung über SYS.1.RESET
- LED-Rückmeldelogik mit UI-Statusfarben

### RS485 auf Waveshare 4.3B
- UART: Serial1
- RX: GPIO43
- TX: GPIO44
- Baudrate: 57600
- Format: 8E1

### Instanz-Mapping in Waveshare43b
- UI-Buttons 1-6 senden BTN-Instanzen 17-22
- LED-Rückmeldungen 49-54 werden auf UI-Buttons 1-6 gemappt
- Beim Drücken wird STATUS.1 gesendet, beim Loslassen STATUS.0

### Build und Upload
Im Ordner Waveshare43b:

1. pio run -e esp32s3
2. pio run -e esp32s3 -t upload --upload-port COM16

## 🚀 Funktionen
*   **Multi-Ebenen-Steuerung:** Verwaltung von drei verschiedenen Ebenen (z.B. Grundfunktionen, EG, OG) mit jeweils eigenen Device-IDs.
*   **Persistente Speicherung:** Speicherung der Device-IDs im ESP32 NVS (Preferences), sodass Einstellungen nach einem Neustart erhalten bleiben.
*   **Hausbus-Protokoll:** Integrierte Kommunikation über RS485 mit automatischem Frame-Handling (Start/End-Bytes, Backoff-Logik).
*   **Dynamische UI:** 
    *   Anzeige von Button-Status (Lokal vs. Remote).
    *   Echtzeit-Update der Button-Labels via Hausbus-Telegramme.
    *   Dropdown-Menü zur Auswahl der aktiven Ebene.
*   **Service-Modus:** Eine dedizierte Konfigurationsseite zum Ändern von Device-IDs und zur Verwaltung der Systemparameter.

## 🛠 Technische Details

### Hardware-Anbindung
*   **Kommunikation:** RS485 über UART (Serial2).
*   **Pins (Standard):** RX = 35, TX = 22.
*   **Baudrate:** 57600 (Format: 8E1).

Hinweis: Diese Pin-Angaben gelten für die CYD-Variante. Die Waveshare 4.3B-Variante nutzt die oben dokumentierten Pins im Ordner Waveshare43b.

### Hausbus-Protokoll Struktur
Die Kommunikation basiert auf einem Punkt-Trenner-Schema innerhalb eines Frames:
`[START_BYTE] DEVICE_ID . FUNCTION . INSTANCE_ID . ACTION . VALUE [END_BYTE]`
*   **Start-Byte:** `0xFD`
*   **End-Byte:** `0xFE`

### Software-Architektur
1.  **Hausbus_kernal:** Die Kernschicht für die serielle Kommunikation. Sie kümmert sich um das Polling des Buffers, das Parsen der Telegramme in eine `HausbusTelegram`-Struktur und das Senden von Befehlen mit einer "Bus Busy"-Prüfung (Backoff-Algorithmus).
2.  **UI & Logik:** Nutzt die LVGL-Bibliothek zur Darstellung. Die Logik trennt zwischen lokalen Benutzerinteraktionen (Grüne Bestätigung) und Remote-Befehlen (Orange Bestätigung).

## 📂 Dateistruktur
*   `Hausbus_kernal.h`: Definition der Datenstrukturen (`HausbusTelegram`) und der Kernfunktionen für die Kommunikation.
*   `Hausbus_kernal.cpp`: Implementierung des Kommunikations-Stacks, inklusive Parsing-Logik und Fehlerbehandlung (Timeout/Backoff).
*   `Hausbus_CYD_V1_03.txt`: Hauptanwendungscode. Enthält:
    *   NVS-Management für die Speicherung von IDs.
    *   LED-Telegramm-Handler zur Zustandsaktualisierung.
    *   Konfigurations-Logik für Button-Labels und Dropdown-Optionen.
    *   Event-Handling für die Touch-Oberfläche.

## ⚙️ Installation & Konfiguration
1.  **Hardware anschließen:** Verbinde den RS485-Konverter mit den Pins 35 (RX) und 22 (TX).
2.  **Bibliotheken:** Stelle sicher, dass `LVGL`, `Preferences` und die entsprechenden Treiber für das CYD Display installiert sind.
3.  **Kompilieren:** Lade das Projekt auf einen ESP32 hoch.
4.  **Initialisierung:** Beim ersten Start werden Standard-IDs (9061, 9062, 9063) verwendet. Diese können über die "Service"-Seite im Menü angepasst werden.

## 🎨 UI Farben & Status
*   **Standard/Released:** Lila (`0xA021F3`)
*   **Lokal gedrückt:** Grün (`0x00C853`)
*   **Remote Befehl:** Orange (`0xFFA500`)
*   **Rahmen (Aktiv):** Grün (`0x009624`)

## 📝 Lizenz
Dieses Projekt ist Teil der Hausbus-Entwicklung. Details zur Lizenz finden Sie in den entsprechenden Dateien oder im Repository-Header.
