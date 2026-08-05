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

Libraries

This project requires the following Arduino libraries:

* SmartTM1637
* DHT Sensor Library
* RTClib
* SD
* SPI

Install them using the Arduino Library Manager before compiling.

⸻

Data Logging

Each successful reading is stored on the SD card with:

* Date
* Time
* Temperature (°C)
* Humidity (%RH)

Example:

2026-08-05,14:35:10,29.6,71.8

⸻

DHT22 Retry Mechanism

Version 1.2.2 introduces a non-blocking retry system.

Workflow:

1. Start sensor reading.
2. If reading succeeds:
    * Save data.
    * Update display.
3. If reading fails:
    * Wait for retry interval.
    * Retry automatically.
4. Maximum retries: 3
5. If all retries fail:
    * Skip the sample.
    * Continue normal operation.

This prevents temporary sensor errors from stopping the logger.

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

Module	Signal	Arduino Pin	Notes
DHT22	DATA	D8	Temperature & Humidity Sensor
TM1637 Display #1	CLK	D2	Display Clock
TM1637 Display #1	DIO	D3	Display Data
TM1637 Display #2	CLK	D4	Display Clock
TM1637 Display #2	DIO	D5	Display Data
TM1637 Display #3	CLK	D6	Display Clock
TM1637 Display #3	DIO	D7	Display Data
MicroSD Module	CS	D10	Chip Select
MicroSD Module	MOSI	D11	Hardware SPI
MicroSD Module	MISO	D12	Hardware SPI
MicroSD Module	SCK	D13	Hardware SPI
DS3231 RTC	SDA	A4	I²C Bus
DS3231 RTC	SCL	A5	I²C Bus

Note

The DS3231 RTC communicates over the I²C bus (A4/A5 on Arduino Uno/Nano), while the MicroSD module uses the hardware SPI interface (D10–D13).

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
⸻

Version History

v1.2.2

* Added non-blocking DHT22 retry handling
* Added maximum three retry attempts
* Improved sensor reliability
* Improved code organization
* Improved main loop responsiveness

v1.2.1

* RTC integration
* SD card logging
* TM1637 display
* DHT22 sensor support

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
