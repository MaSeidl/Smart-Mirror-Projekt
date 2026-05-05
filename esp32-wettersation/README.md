# 🌦️ ESP32 Wetterstation (MQTT)

Dieses Projekt ist eine Wetterstation basierend auf einem ESP32.
Die gemessenen Umweltdaten werden erfasst und per MQTT an einen Broker (z. B. Raspberry Pi mit Mosquitto) gesendet.

---

## 🚀 Features

* 🌡️ Temperatur (BME280)
* 💧 Luftfeuchtigkeit (BME280)
* 🌬️ Luftdruck (BME280, auf Meereshöhe korrigiert)
* 🌧️ Regenmessung über Kippwaage (Interrupt)
* 💨 Windgeschwindigkeit über ADC (analoger Windsensor)
* 📡 MQTT-Übertragung
* 🖥️ Serielle Debug-Ausgabe
* 🔄 MQTT Reconnect + Last-Will

---

## 🛠️ Verwendete Hardware

* ESP32
* BME280 (I²C, Adresse 0x76)
* Regensensor (Kippimpuls)
* Windsensor (Analog, Spannungsausgang)

---

## 🔌 Pinbelegung

| Funktion    | GPIO   |
| ----------- | ------ |
| I2C SDA     | GPIO4  |
| I2C SCL     | GPIO5  |
| Regensensor | GPIO27 |
| Windsensor  | GPIO2  |

---

## ⚙️ Wichtige Parameter (aus dem Code)

| Parameter               | Wert      |
| ----------------------- | --------- |
| I2C Adresse             | 0x76      |
| I2C Takt                | 100 kHz   |
| Höhe (für Druck)        | 373 m     |
| Regen pro Impuls        | 0.2794 mm |
| Max Windspannung        | 3.8 V     |
| Max Windgeschwindigkeit | 32.4 m/s  |

---

## ⚙️ Software / Voraussetzungen

* ESP-IDF (v5.5.2 empfohlen)
* CMake
* Python

---

## 🔌 Installation & Flashen

```bash
idf.py build
idf.py flash
idf.py monitor
```

---

## 🔧 Konfiguration

⚠️ Aktuell sind WLAN und MQTT **fest im Code definiert**.

```c
#define WIFI_SSID "ArduinoWiFi"
#define WIFI_PASS "2-bszam!"
#define MQTT_BROKER "mqtt://172.16.8.13"
```

👉 Für ein sauberes Projekt sollte das später in eine `config.h` ausgelagert werden.

---

## 📡 MQTT Topics

| Messwert   | Topic                                   |
| ---------- | --------------------------------------- |
| Temperatur | BSZAM/Wetterstation/Temperatur          |
| Luftdruck  | BSZAM/Wetterstation/Luftdruck           |
| Feuchte    | BSZAM/Wetterstation/Luftfeuchtigkeit    |
| Regen      | BSZAM/Wetterstation/Regen               |
| Wind       | BSZAM/Wetterstation/Windgeschwindigkeit |
| Status     | BSZAM/Wetterstation/Systemstatus        |

---

## 📟 Beispiel Ausgabe

```
Temp 22.50 °C | Druck 1013.20 hPa | Feuchte 45.00 % | Regen 0.00 l/m² (NEIN) | Wind 3.20 km/h | Status Ein
```

---

## ⚠️ Hinweise

* ESP32 und MQTT-Broker müssen im selben Netzwerk sein
* MQTT läuft typischerweise auf einem Raspberry Pi (Mosquitto)
* Windsensor nutzt ADC (max. 3.3 V beachten!)
* Regensensor nutzt Interrupt mit Entprellung (50 ms)

---

## 📌 TODO

* [ ] WLAN & MQTT in config.h auslagern
* [ ] Schaltplan hinzufügen
* [ ] Kalibrierung Windsensor verbessern
* [ ] Deep Sleep / Energiesparen
* [ ] Fehlerhandling erweitern

---

## 📄 Lizenz

Freie Nutzung für private und nicht-kommerzielle Projekte.
