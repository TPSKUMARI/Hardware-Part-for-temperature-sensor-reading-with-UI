# Temperature Sensor with UI

An ESP32-S3 project that reads temperature from a TMP117 sensor and displays it on a live web dashboard — no app or internet required.

## How it works

- The ESP32 reads temperature over I2C from the TMP117 sensor.
- It creates its own WiFi access point (`Temperature_Monitor`, open/no password).
- Connecting to that network auto-opens a captive portal showing the live temperature, min/max stats, and a comfort status (cold/cool/comfortable/warm/hot).
- The page polls the sensor every second and can reset min/max readings with a button.

## Hardware

- ESP32-S3
- Adafruit TMP117 temperature sensor (I2C)

**Wiring:**

| TMP117 | ESP32-S3 |
|--------|----------|
| VCC    | 3.3V     |
| GND    | GND      |
| SDA    | GPIO 21  |
| SCL    | GPIO 22  |

## Project structure

- [Temperature Sensor/temperaure_reading/temperaure_reading.ino](Temperature%20Sensor/temperaure_reading/temperaure_reading.ino) — minimal sketch, prints temperature to Serial only.
- [Temperature Sensor/temperaure_reading/with_ui_AP/with_ui_AP.ino](Temperature%20Sensor/temperaure_reading/with_ui_AP/with_ui_AP.ino) — full version with WiFi AP, captive portal, and web UI.

## Getting started

1. Open the desired `.ino` file in the Arduino IDE (with ESP32 board support installed).
2. Install the `Adafruit_TMP117` and `Adafruit_Sensor` libraries.
3. Wire the TMP117 as shown above.
4. Flash to the ESP32-S3.
5. For the UI version, connect to the `Temperature_Monitor` WiFi network and the dashboard should open automatically (or browse to `http://192.168.4.1`).
