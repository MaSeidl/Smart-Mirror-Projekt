# Smart-Mirror-Projekt
Ein selbstentwickelter Smart Mirror zeigt auf Basis einen Raspberry Pi neben News auch die Wetterdaten der eigenen Wetterstation. Außerdem soll eine Konfiguration über das Smart Phone möglich sein.


# Smart Mirror Wetterstation

## Beschreibung
Dieses Projekt ist Teil eines Smart-Mirror-Systems.  
Eine Wetterstation erfasst Umgebungsdaten und überträgt diese per MQTT an einen Raspberry Pi.

## Funktionen
- Temperatur, Luftdruck und Luftfeuchtigkeit (BME280)
- Windgeschwindigkeit
- Niederschlag (Regensensor)
- Datenübertragung über WLAN (MQTT)

## Aufbau
- ESP32-C5 als Mikrocontroller
- Sensoren über I²C und ADC angebunden
- Raspberry Pi mit MagicMirror zur Anzeige

## Installation

### Voraussetzungen
- ESP-IDF installiert
- Python
- WLAN Netzwerk

### Schritte
```bash
git clone <repo>
cd esp32-wetterstation
idf.py build
idf.py flash monitor