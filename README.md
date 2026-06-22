Hausbus CYD Controller (V1.02/1.03)
Dieses Projekt implementiert eine Benutzeroberfläche für einen CYD (Cheap Yellow Display), der als Steuerungszentrale für ein Smart-Home-System über das Hausbus-Protokoll fungiert. Das System ermöglicht die Steuerung von LEDs und anderen Geräten via RS485/UART sowie die Konfiguration von Geräte-IDs und Button-Labels über eine grafische Oberfläche (LVGL).

🚀 Funktionen
Multi-Ebenen-Steuerung: Verwaltung von drei verschiedenen Ebenen (z.B. Grundfunktionen, EG, OG) mit jeweils eigenen Device-IDs.
Persistente Speicherung: Speicherung der Device-IDs im ESP32 NVS (Preferences), sodass Einstellungen nach einem Neustart erhalten bleiben.
Hausbus-Protokoll: Integrierte Kommunikation über RS485 mit automatischem Frame-Handling (Start/End-Bytes, Backoff-Logik).
Dynamische UI:
Anzeige von Button-Status (Lokal vs. Remote).
Echtzeit-Update der Button-Labels via Hausbus-Telegramme.
Dropdown-Menü zur Auswahl der aktiven Ebene.
Service-Modus: Eine dedizierte Konfigurationsseite zum Ändern von Device-IDs und zur Verwaltung der Systemparameter.
🛠 Technische Details
Hardware-Anbindung
Kommunikation: RS485 über UART (Serial2).
Pins (Standard): RX = 35, TX = 22.
Baudrate: 57600 (Format: 8E1).
Hausbus-Protokoll Struktur
Die Kommunikation basiert auf einem Punkt-Trenner-Schema innerhalb eines Frames:
[START_BYTE] DEVICE_ID . FUNCTION . INSTANCE_ID . ACTION . VALUE [END_BYTE]

Start-Byte: 0xFD
End-Byte: 0xFE
Software-Architektur
Hausbus_kernal: Die Kernschicht für die serielle Kommunikation. Sie kümmert sich um das Polling des Buffers, das Parsen der Telegramme in eine HausbusTelegram-Struktur und das Senden von Befehlen mit einer "Bus Busy"-Prüfung (Backoff-Algorithmus).
UI & Logik: Nutzt die LVGL-Bibliothek zur Darstellung. Die Logik trennt zwischen lokalen Benutzerinteraktionen (Grüne Bestätigung) und Remote-Befehlen (Orange Bestätigung).
📂 Dateistruktur
Hausbus_kernal.h: Definition der Datenstrukturen (HausbusTelegram) und der Kernfunktionen für die Kommunikation.
Hausbus_kernal.cpp: Implementierung des Kommunikations-Stacks, inklusive Parsing-Logik und Fehlerbehandlung (Timeout/Backoff).
Hausbus_CYD_V1_03.txt: Hauptanwendungscode. Enthält:
NVS-Management für die Speicherung von IDs.
LED-Telegramm-Handler zur Zustandsaktualisierung.
Konfigurations-Logik für Button-Labels und Dropdown-Optionen.
Event-Handling für die Touch-Oberfläche.
⚙️ Installation & Konfiguration
Hardware anschließen: Verbinde den RS485-Konverter mit den Pins 35 (RX) und 22 (TX).
Bibliotheken: Stelle sicher, dass LVGL, Preferences und die entsprechenden Treiber für das CYD Display installiert sind.
Kompilieren: Lade das Projekt auf einen ESP32 hoch.
Initialisierung: Beim ersten Start werden Standard-IDs (9061, 9062, 9063) verwendet. Diese können über die "Service"-Seite im Menü angepasst werden.
🎨 UI Farben & Status
Standard/Released: Lila (0xA021F3)
Lokal gedrückt: Grün (0x00C853)
Remote Befehl: Orange (0xFFA500)
Rahmen (Aktiv): Grün (0x009624)
