# MiniKate

MiniKate ist eine ESP32-basierte mobile Roboterplattform mit zwei geregelten Antriebsmotoren, Linienverfolgung, Hinderniserkennung, RFID, Akkuüberwachung, LED- und Soundeffekten sowie WLAN-Kommunikation. Die Firmware unterstützt autonome Fahrmodi, eine RemoteXY-Fernsteuerung und eine lokale Weboberfläche für Konfiguration und Firmware-Updates.

## Funktionen

- Zwei Gleichstrommotoren mit Encodern und Geschwindigkeitsregelung
- Fahrmodi: Idle, Linie, frei, Spiel, Drehen, Vorwärts und Rückwärts
- Linienerkennung über drei Sensoren
- Hinderniserkennung per Ultraschallsensor
- RFID-Erkennung mit MFRC522
- Akkuüberwachung mit einstellbarem Kalibrierungsfaktor
- BMI160-IMU für Fahrtrichtung und Drehungen
- Status- und Befehlsaustausch über WLAN/TCP und JSON
- RemoteXY-Fernsteuerung über bestehende WLAN-Verbindung
- Lokale Weboberfläche für Einstellungen und OTA-Firmwareupload
- NeoPixel-Animationen, Blinker und Piezo-Signale
- Speicherung der Konfiguration im ESP32-NVS

## Hardware

- ESP32
- Zwei Motoren mit H-Brücke und Encodern
- DFRobot BMI160
- Drei digitale Liniensensoren
- HC-SR04 oder kompatibler Ultraschallsensor
- MFRC522 RFID-Leser
- Sechs adressierbare NeoPixel-LEDs
- Piezo-Summer
- Batteriespannungsteiler

## Pinbelegung

| Funktion | ESP32-Pin |
|---|---:|
| Motor rechts vorwärts | 14 |
| Motor rechts rückwärts | 13 |
| Motor rechts PWM | 12 |
| Encoder rechts | 17 |
| Motor links vorwärts | 27 |
| Motor links rückwärts | 26 |
| Motor links PWM | 25 |
| Encoder links | 16 |
| Liniensensor links | 33 |
| Liniensensor Mitte | 34 |
| Liniensensor rechts | 35 |
| Ultraschall Trigger | 5 |
| Ultraschall Echo | 4 |
| I²C SDA | 21 |
| I²C SCL | 22 |
| RFID SS | 2 |
| RFID Reset | 0 |
| RFID SCK | 18 |
| RFID MISO | 19 |
| RFID MOSI | 23 |
| NeoPixel-Daten | 15 |
| Piezo | 32 |
| Batteriespannung | 39 |

Die zentrale Hardwarekonfiguration befindet sich in [`Config.h`](Config.h).

## Benötigte Bibliotheken

Neben dem ESP32-Arduino-Core werden folgende Bibliotheken benötigt:

- ArduinoJson
- Adafruit NeoPixel
- MFRC522
- DFRobot BMI160
- RemoteXY

`WiFi`, `WebServer`, `Update`, `Preferences`, `SPI` und `Wire` sind Bestandteil des ESP32-Arduino-Cores.

## Installation

1. Repository klonen oder als ZIP herunterladen.
2. Den Ordner in der Arduino IDE öffnen und `MiniKate.ino` auswählen.
3. Das passende ESP32-Board installieren und auswählen.
4. Die genannten Bibliotheken über den Bibliotheksverwalter installieren.
5. Pinbelegung und Hardwarewerte in `Config.h` prüfen.
6. Sketch kompilieren und auf den ESP32 übertragen.

## Erste Einrichtung

Sind noch keine WLAN-Zugangsdaten gespeichert, startet MiniKate einen Fallback-Access-Point. Über die lokale Weboberfläche können unter anderem folgende Werte gespeichert werden:

- Roboter-ID und Name
- WLAN-SSID und Passwort
- Serveradresse und Port
- Akku-Kalibrierungsfaktor
- Motorlimits links und rechts

Die Einstellungen werden mit `Preferences` dauerhaft im Namespace `minikate` gespeichert.

## Kommunikation

Die Klasse `Connection` verwaltet WLAN, TCP-Kommunikation und JSON-Nachrichten. MiniKate sendet regelmäßig Statusinformationen wie Akkuspannung, Fahrstatus, Hindernisabstand und Richtung. RFID-Ereignisse und weitere Modulereignisse werden ebenfalls an den konfigurierten Server übertragen.

## Projektstruktur

| Datei | Aufgabe |
|---|---|
| `MiniKate.ino` | Hauptprogramm und Modulkoordination |
| `Config.h` | Pins und Hardwarekonstanten |
| `Drive.*` | Fahrmodi, IMU und Bewegungssteuerung |
| `Motor.*` | Motor-, Encoder- und Geschwindigkeitsregelung |
| `Perception.*` | RFID, Akku und Ultraschall |
| `Connection.*` | WLAN, TCP, JSON und RemoteXY |
| `Effects.*` | LEDs, Animationen, Blinker und Piezo |
| `Interface.*` | Webkonfiguration und Firmwareupload |
| `Settings.*` | Dauerhafte Konfiguration |
| `RemoteXYNet_WiFiExisting.h` | RemoteXY über bestehende WLAN-Verbindung |

## Hinweise

- Motoren und LEDs sollten nicht direkt aus dem ESP32-Regler versorgt werden.
- Alle Komponenten benötigen eine gemeinsame Masse.
- Die Akkumessung setzt einen geeigneten Spannungsteiler voraus. Der Standardfaktor ist projektspezifisch und sollte mit einem Multimeter kalibriert werden.
- Vor dem ersten Fahrtest sollte das Fahrzeug aufgebockt und die Motorlaufrichtung geprüft werden.
- Das Repository enthält keine kompilierten Firmwaredateien oder IDE-Build-Artefakte.

## Lizenz

Für dieses Projekt ist derzeit keine Lizenzdatei hinterlegt. Ohne ausdrückliche Lizenz bleiben alle Rechte beim Urheber.
