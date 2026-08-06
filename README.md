# temp_humi_logger

Temp/Humi Logger v1.2.2

A reliable Arduino-based Temperature & Humidity Data Logger featuring a DHT22 sensor, DS3231 Real-Time Clock (RTC), MicroSD card logging, and a TM1637 4-digit display.

Version 1.2.2 introduces a more robust, non-blocking DHT22 reading mechanism with automatic retry handling, improving stability without interrupting the main program.

⸻

Features

* 🌡️ Temperature measurement using DHT22
* 💧 Relative humidity measurement
* 🕒 Accurate timestamps using DS3231 RTC
* 💾 Automatic logging to MicroSD card
* 📟 Live data display on TM1637 4-digit display
* ⚡ Non-blocking program architecture using millis()
* 🔁 Automatic DHT22 retry (maximum 3 attempts)
* 📂 CSV-compatible log files
* 🛡️ Improved reliability during sensor read failures

⸻

What’s New in v1.2.2

Added

* Non-blocking DHT22 retry handling
* Maximum of three retry attempts
* Retry delay without stopping the main loop
* Cleaner task separation
* Better program responsiveness

Improved

* Overall code structure
* Sensor fault tolerance
* Main loop readability
* Easier future expansion

⸻

Hardware

* Arduino Uno / Nano (or compatible)
* DHT22 Temperature & Humidity Sensor
* DS3231 RTC Module
* MicroSD Card Module
* TM1637 4-Digit Display
* MicroSD Card
* Jumper wires
* 5V power supply

⸻

## Libraries

This project requires the following Arduino libraries:

- SmartTM1637
- MyDHT22
- RTClib
- SdFat
- Wire (built into the Arduino IDE)

Install the required third-party libraries using the Arduino Library Manager before compiling.
⸻

## Data Logging

Each reading is automatically stored on the SD card in CSV format.

The log file contains:

- Date
- Time
- Temperature (°C)
- Humidity (%RH)
- DHT22 status
- RTC status
- SD card status
- Display #1 status
- Display #2 status
- Supply voltage (VCC)

Example:

```text
2026-08-05,14:35:10,29.6,71.8,1,1,1,1,1,4.97
```

If a DHT22 reading fails after all retry attempts, the logger records:

```text
2026-08-05,14:35:10,NA,NA,0,1,1,0,0,4.97
```

⸻

## DHT22 Retry Mechanism

Version 1.2.2 introduces a non-blocking retry system.

Workflow:

1. Start a sensor reading session.
2. If the reading succeeds:
   - Update the displays.
   - Save the measurement to the SD card.
3. If the reading fails:
   - Wait for the configured retry interval.
   - Retry automatically.
4. Maximum retry attempts: **3**.
5. If all retry attempts fail:
   - Record `NA` for Temperature and Humidity.
   - Set `DHT_OK = 0` in the log file.
   - Continue normal operation without blocking the main loop.

This approach improves long-term reliability by allowing the logger to recover from temporary sensor communication errors while preserving a complete log history.

⸻

Advantages

* No delay() blocking
* Better multitasking
* Stable long-term logging
* Easier debugging
* Easier maintenance
* Ready for future feature expansion

⸻

Possible Future Improvements

* Serial command interface
* OLED/TFT display support
* Wi-Fi logging (ESP32)
* MQTT support
* Web dashboard
* OTA firmware updates
* Battery monitoring
* Alarm thresholds
* Data statistics
* USB data export

⸻

Pin Configuration


            +----------------------+
            |   Arduino Uno/Nano   |
            +----------------------+
      D2  ----------------> TM1637 #1 CLK
      D3  ----------------> TM1637 #1 DIO

      D4  ----------------> TM1637 #2 CLK
      D5  ----------------> TM1637 #2 DIO

      D6  ----------------> TM1637 #3 CLK
      D7  ----------------> TM1637 #3 DIO

      D8  ----------------> DHT22 DATA

      D10 ----------------> SD Card CS
      D11 ----------------> SD Card MOSI
      D12 ----------------> SD Card MISO
      D13 ----------------> SD Card SCK

      A4  ----------------> DS3231 SDA
      A5  ----------------> DS3231 SCL

Note

The DS3231 RTC communicates over the I²C bus (A4/A5 on Arduino Uno/Nano), while the MicroSD module uses the hardware SPI interface (D10–D13).

## Wiring Diagram

![Arduino Nano Wiring Diagram](docs/wiring/arduino_nano_wiring_diagram.png)
⸻

## Version History

### v1.2.2

- Refactored the non-blocking DHT22 retry mechanism
- Improved code organization and readability
- Improved retry state handling
- Added safer VCC measurement
- Updated CSV header names
- Improved long-term reliability

### v1.2.1

- Introduced DHT22 retry handling
- Added up to three retry attempts
- Improved sensor read reliability

### v1.2.0

- Added supply voltage (VCC) monitoring
- Added display status logging
- Improved SD card logging structure

⸻

License

This project is released under the MIT License.

You are free to use, modify, and distribute this project under the terms of the license.

⸻

Author

Fadhil Hashim

Embedded Systems • Arduino • Automotive Electronics • Data Logging

⸻

Contributing

Contributions, bug reports, and feature suggestions are welcome.

If you find this project useful, consider giving it a ⭐ on GitHub.
