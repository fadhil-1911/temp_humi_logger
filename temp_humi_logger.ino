//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  Temp/Humi Logger
//                   Version: bme(fix freeze)
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
  Project : Temp/Humi Logger with RTC + DHT22 + SD + TM1637
  Version : xxx
  Date    : 2026-08-17

  Reliability updates:
  - Added 8-second hardware watchdog recovery.
  - Added periodic watchdog reset in the main loop.
  - Added I2C timeout protection.
  - Reduced I2C bus speed to 100 kHz for improved stability.
  - Limited RTC polling frequency to reduce I2C traffic.
  - Moved the heartbeat LED to D9 to avoid SPI SCK conflict on D13.
  - Added low-VCC warning below 4.5 V.
  - Log "NA" when the RTC timestamp is invalid.
  - Preserve RTC, DHT22, SD, and display status flags in the CSV log.
*/

//==========================================================
// Libraries
//==========================================================
#include <SmartTM1637.h>  // TM1637 display control
#include <Wire.h>         // I2C communication
#include <RTClib.h>       // DS3231 RTC support
#include <SdFat.h>        // SD card file system
#include <avr/wdt.h>      // AVR hardware watchdog timer

//==========================================================
// Pin Configuration
//==========================================================
#define HEARTBEAT_LED 9  // External heartbeat LED
#define DHT_PIN 8        // DHT22 data pin

#define CLK_PIN_1 2  // TM1637 display 1 clock pin
#define DIO_PIN_1 3  // TM1637 display 1 data pin

#define CLK_PIN_2 4  // TM1637 display 2 clock pin
#define DIO_PIN_2 5  // TM1637 display 2 data pin

#define CLK_PIN_3 6  // TM1637 display 3 clock pin
#define DIO_PIN_3 7  // TM1637 display 3 data pin

#define CS_PIN 10  // SD card SPI chip-select pin

#define VERSION "v000"  // Firmware version shown during startup

// SET SENSOR DHT22 OR SHT41
#define SENSOR_DHT22
//#define SENSOR_SHT41

#ifdef SENSOR_DHT22
#include <MyDHT22.h>
#endif

#ifdef SENSOR_SHT41
#include <Adafruit_Sensor.h>
#include <Adafruit_SHT4x.h>
#endif

//==========================================================
// Device Objects
//==========================================================
SmartTM1637 display_1(CLK_PIN_1, DIO_PIN_1);  // Temperature display
SmartTM1637 display_2(CLK_PIN_2, DIO_PIN_2);  // Humidity display
SmartTM1637 display_3(CLK_PIN_3, DIO_PIN_3);  // RTC clock display

//MyDHT22 dht(DHT_PIN);  // DHT22 sensor object
RTC_DS3231 rtc;  // DS3231 RTC object
SdFat SD;        // SD card file-system object
File logFile;    // SD log file object


#ifdef SENSOR_DHT22
MyDHT22 dht(DHT_PIN);
#endif

#ifdef SENSOR_SHT41
Adafruit_SHT4x sht4;
#endif

//==========================================================
// SENSOR Timing
//==========================================================
unsigned long lastReadTimeSensor = 0;  // Time of the last completed SENSOR session

//==========================================================
// RTC Display Timing
//==========================================================
unsigned long lastClockUpdate = 0;              // Last clock display update time
const unsigned long clockUpdateInterval = 250;  // Update clock display every 250 ms

//==========================================================
// Heartbeat Timing
//==========================================================
unsigned long lastHeartbeat = 0;  // Last heartbeat toggle time
bool heartbeatState = false;      // Current heartbeat LED state

//==========================================================
// SENSOR Non-Blocking Retry State
//==========================================================
const int maxRetry = 3;                    // Maximum SENSOR read attempts
int retryCount = 0;                        // Current retry count
unsigned long lastRetryTime = 0;           // Time of the previous retry attempt
const unsigned long retryInterval = 2000;  // Delay between retry attempts

bool readOK = false;           // True when the latest SENSOR read succeeds
bool retryInProgress = false;  // True while a retry session is active
bool resultReady = false;      // True when a SENSOR session has completed

float temp = 0.0;  // Latest valid temperature value
float humi = 0.0;  // Latest valid humidity value

//==========================================================
// Device Status Flags
//==========================================================
bool sensorStatus = false;  // SENSOR read status
bool rtcStatus = false;     // RTC read status
bool sdStatus = false;      // SD card status
bool disp1Status = false;   // Temperature display status
bool disp2Status = false;   // Humidity display status

//==========================================================
// Setup
//==========================================================
void setup() {

  // Disable the watchdog during system initialization.
  wdt_disable();

  // Initialize serial communication for diagnostics.
  Serial.begin(9600);

  // Initialize all TM1637 displays.
  display_1.begin(4);
  display_2.begin(4);
  display_3.begin(1);

  // Initialize the I2C bus.
  Wire.begin();

  // Use standard-mode I2C for improved long-run stability.
  Wire.setClock(100000);

  // Abort and reset the TWI hardware if an I2C transaction
  // remains stuck for more than 25 ms.
  Wire.setWireTimeout(25000, true);

  // Initialize the DS3231 RTC.
  if (!rtc.begin()) {
    Serial.println("RTC initialization failed");
  }

  // Show startup information on the displays.
  display_1.print("init");
  display_2.print(VERSION, false, true, true, false);

  // Configure the external heartbeat LED.
  pinMode(HEARTBEAT_LED, OUTPUT);

  // RTC adjustment:
  // Uncomment once when the RTC must be synchronized with compile time.
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  //========================================================
  // SD Card Initialization
  //========================================================
  sdStatus = SD.begin(CS_PIN);

  if (!sdStatus) {
    Serial.println("❌ Failed to initialize SD card!");
    display_1.print("SdEr");
  } else {
    Serial.println("✅ SD Card OK");
  }

  //========================================================
  // Create CSV Header
  //========================================================
  // Create log.csv only when it does not already exist.
  if (!SD.exists("log.csv")) {
    logFile = SD.open("log.csv", FILE_WRITE);

    if (logFile) {
      logFile.println(
        "Date,Time,Temp_C,Humi_PCT,"
        "SENSOR_OK,RTC_OK,SD_OK,DISP1_OK,DISP2_OK,VCC_V");

      logFile.close();
    }
  }



#ifdef SENSOR_DHT22
  Serial.println(F("Sensor: DHT22"));
#endif

#ifdef SENSOR_SHT41
  Serial.println(F("Sensor: SHT41"));

  sensorStatus = sht4.begin();

  if (!sensorStatus) {
    Serial.println(F("SHT41 initialization failed!"));
  } else {
    Serial.println(F("SHT41 initialization OK"));
  }
#endif

  // Allow startup messages to remain visible before normal operation.
  delay(3000);

  // Enable the hardware watchdog.
  // The MCU will reset if the main loop stops servicing it for 8 seconds.
  wdt_enable(WDTO_8S);
}

//==========================================================
// Main Loop
//==========================================================
void loop() {

  // Confirm that the main loop is still running.
  // If execution becomes blocked for more than 8 seconds,
  // the watchdog will automatically reset the MCU.
  wdt_reset();

  // Capture millis() once and reuse it throughout this loop iteration.
  unsigned long currentMillis = millis();

  //========================================================
  // 1. Start a New SENSOR Reading Session
  //========================================================
  // Begin a new session when:
  // - no retry session is active,
  // - no completed result is waiting to be processed,
  // - at least 2 seconds have passed since the previous session.
  if (!retryInProgress && !resultReady && currentMillis - lastReadTimeSensor >= 2000) {

    // Record the start time of this reading session.
    lastReadTimeSensor = currentMillis;

    // Reset the retry and result states.
    retryCount = 0;
    readOK = false;
    sensorStatus = false;
    disp1Status = false;
    disp2Status = false;

    // Mark the retry state machine as active.
    retryInProgress = true;

    // Backdate lastRetryTime so the first read attempt
    // can execute immediately without waiting.
    lastRetryTime = currentMillis - retryInterval;
  }

  //========================================================
  // 2. Process SENSOR Retry Attempts
  //========================================================
  // Attempt another SENSOR read when the retry interval has elapsed.
  if (retryInProgress && currentMillis - lastRetryTime >= retryInterval) {

    lastRetryTime = currentMillis;

#ifdef SENSOR_DHT22

    if (dht.readData()) {
      temp = dht.getTemperature();
      humi = dht.getHumidity();

      sensorStatus = true;
      readOK = true;
      retryInProgress = false;
      resultReady = true;

    } else {
      retryCount++;

      Serial.print(F("DHT22 reading attempt failed: "));
      Serial.print(retryCount);
      Serial.print(F("/"));
      Serial.println(maxRetry);

      if (retryCount >= maxRetry) {
        sensorStatus = false;
        readOK = false;
        retryInProgress = false;
        resultReady = true;
      }
    }

#endif


#ifdef SENSOR_SHT41

    sensors_event_t humidity, temperature;

    sht4.getEvent(&humidity, &temperature);

    temp = temperature.temperature;
    humi = humidity.relative_humidity;

    if (!isnan(temp) && !isnan(humi)) {
      sensorStatus = true;
      readOK = true;
      retryInProgress = false;
      resultReady = true;

    } else {
      retryCount++;

      Serial.print(F("SHT41 reading attempt failed: "));
      Serial.print(retryCount);
      Serial.print(F("/"));
      Serial.println(maxRetry);

      if (retryCount >= maxRetry) {
        sensorStatus = false;
        readOK = false;
        retryInProgress = false;
        resultReady = true;
      }
    }

#endif
  }

  //========================================================
  // 3. Process the Completed Sensor Result
  //========================================================
  if (resultReady) {

    // Clear the result flag so this block runs only once per session.
    resultReady = false;

    // Schedule the next SENSOR reading session relative to now.
    lastReadTimeSensor = currentMillis;

    //======================================================
    // 3.1 Read RTC Timestamp
    //======================================================
    DateTime now = rtc.now();

    // Check whether the RTC transaction caused an I2C timeout.
    if (Wire.getWireTimeoutFlag()) {
      Serial.println("WARNING: I2C timeout during RTC log read");

      // Clear the timeout flag so future transactions can be monitored.
      Wire.clearWireTimeoutFlag();

      // Mark the RTC timestamp as invalid.
      rtcStatus = false;

    } else {

      // Perform a basic validity check on the RTC year.
      rtcStatus = (now.year() >= 2020);
    }

    // Allocate fixed-size buffers for CSV date and time strings.
    char dateStr[11];
    char timeStr[9];

    // Format the RTC date as YYYY-MM-DD.
    snprintf(
      dateStr,
      sizeof(dateStr),
      "%04d-%02d-%02d",
      now.year(),
      now.month(),
      now.day());

    // Format the RTC time as HH:MM:SS.
    snprintf(
      timeStr,
      sizeof(timeStr),
      "%02d:%02d:%02d",
      now.hour(),
      now.minute(),
      now.second());

    //======================================================
    // 3.2 Measure MCU Supply Voltage
    //======================================================
    // Read VCC only once and reuse the same value
    // for Serial output and SD logging.
    long vccMillivolts = readVcc();
    float vccVolts = vccMillivolts / 1000.0;

    // Warn when the measured 5 V rail drops below 4.5 V.
    if (vccVolts < 4.5) {
      Serial.println(F("WARNING: Low VCC"));
    }

    //======================================================
    // 3.3 Update Temperature and Humidity Displays
    //======================================================
    if (readOK) {

      // Display the latest valid temperature and humidity values.
      display_1.print(temp, "C", true);
      display_2.print(humi, "h", true);

      // Mark both sensor displays as successfully updated.
      disp1Status = true;
      disp2Status = true;

      // Print the current measurement to Serial Monitor.
      Serial.print(F("Temp: "));
      Serial.print(temp, 1);
      Serial.print(F(" | Humidity: "));
      Serial.print(humi, 1);
      Serial.print(F(" | VCC: "));
      Serial.print(vccVolts, 2);
      Serial.println(F(" V"));

    } else {

      // Show an error message when all SENSOR attempts fail.
      display_1.print("Err");
      display_2.print("Err");

      // Mark both sensor displays as invalid for this cycle.
      disp1Status = false;
      disp2Status = false;

      Serial.println(F("❌ Failed to read SENSOR after 3 attempts"));
    }

    //======================================================
    // 3.4 Open CSV Log File
    //======================================================
    logFile = SD.open("log.csv", FILE_WRITE);

    if (logFile) {

      // Mark the SD card as available for this logging cycle.
      sdStatus = true;

      //====================================================
      // Date
      //====================================================
      // Write the RTC date only when the RTC status is valid.
      if (rtcStatus) {
        logFile.print(dateStr);
      } else {
        logFile.print("NA");
      }

      logFile.print(",");

      //====================================================
      // Time
      //====================================================
      // Write the RTC time only when the RTC status is valid.
      if (rtcStatus) {
        logFile.print(timeStr);
      } else {
        logFile.print("NA");
      }

      logFile.print(",");

      //====================================================
      // Temperature
      //====================================================
      // Write temperature only when the SENSOR reading is valid.
      if (sensorStatus) {
        logFile.print(temp, 1);
      } else {
        logFile.print("NA");
      }

      logFile.print(",");

      //====================================================
      // Humidity
      //====================================================
      // Write humidity only when the SENSOR reading is valid.
      if (sensorStatus) {
        logFile.print(humi, 1);
      } else {
        logFile.print("NA");
      }

      //====================================================
      // Device Status Flags
      //====================================================
      logFile.print(",");
      logFile.print(sensorStatus ? "1" : "0");

      logFile.print(",");
      logFile.print(rtcStatus ? "1" : "0");

      logFile.print(",");
      logFile.print(sdStatus ? "1" : "0");

      logFile.print(",");
      logFile.print(disp1Status ? "1" : "0");

      logFile.print(",");
      logFile.print(disp2Status ? "1" : "0");

      //====================================================
      // VCC
      //====================================================
      // Append the measured MCU supply voltage and end the CSV row.
      logFile.print(",");
      logFile.println(vccVolts, 2);

      // Flush the current row and release the file handle.
      logFile.close();

      //====================================================
      // Serial Logging Status
      //====================================================
      if (sensorStatus && rtcStatus) {
        Serial.println(F("✅ Log saved to SD card"));

      } else if (!sensorStatus && !rtcStatus) {
        Serial.println(F("⚠️ SENSOR and RTC failure logged"));

      } else if (!sensorStatus) {
        Serial.println(F("⚠️ SENSOR failure logged"));

      } else {
        Serial.println(F("⚠️ RTC failure logged"));
      }

    } else {

      // Mark the SD card unavailable if the log file cannot be opened.
      sdStatus = false;
      Serial.println(F("❌ Failed to open log.csv"));
    }
  }

  //========================================================
  // 4. Update the RTC Clock Display
  //========================================================
  // Update the clock display every 250 ms instead of reading
  // the RTC continuously on every loop iteration.
  if (currentMillis - lastClockUpdate >= clockUpdateInterval) {

    // Record the time of this display update.
    lastClockUpdate = currentMillis;

    // Read the current RTC time.
    DateTime clockNow = rtc.now();

    // Check whether the RTC read caused an I2C timeout.
    if (Wire.getWireTimeoutFlag()) {
      Serial.println(F("WARNING: I2C timeout"));

      // Clear the timeout flag for future monitoring.
      Wire.clearWireTimeoutFlag();

      // Mark the RTC as unavailable.
      rtcStatus = false;

    } else {

      // RTC communication succeeded.
      rtcStatus = true;

      // Extract hour and minute for the clock display.
      uint8_t hour = clockNow.hour();
      uint8_t minute = clockNow.minute();

      // Blink the clock colon every 500 ms.
      bool showColon = (currentMillis / 500) % 2 == 0;

      // Update the TM1637 clock display.
      display_3.printTime(
        hour,
        minute,
        showColon);
    }
  }

  //========================================================
  // 5. Main Loop Heartbeat
  //========================================================
  // Toggle the external LED every second.
  // A stopped LED indicates that the main loop is no longer progressing.
  if (currentMillis - lastHeartbeat >= 1000) {

    // Save the current heartbeat timestamp.
    lastHeartbeat = currentMillis;

    // Toggle the LED state.
    heartbeatState = !heartbeatState;

    // Apply the new state to the external LED.
    digitalWrite(HEARTBEAT_LED, heartbeatState);
  }
}

//==========================================================
// Internal VCC Monitor
//==========================================================
long readVcc() {

  // Select AVcc as the ADC reference and measure
  // the ATmega328P internal nominal 1.1 V reference.
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

  // Allow the ADC reference and multiplexer to stabilize.
  delay(2);

  // Perform one dummy conversion to stabilize the ADC result.
  ADCSRA |= _BV(ADSC);

  while (bit_is_set(ADCSRA, ADSC)) {
  }

  // Perform the actual ADC conversion.
  ADCSRA |= _BV(ADSC);

  while (bit_is_set(ADCSRA, ADSC)) {
  }

  // Read the completed ADC conversion result.
  uint16_t result = ADC;

  // Avoid division by zero if the ADC returns an invalid result.
  if (result == 0) {
    return 0;
  }

  // Calibrated internal-reference constant used to estimate VCC.
  const long VCC_CALIBRATION = 1111800L;

  // Return the estimated MCU supply voltage in millivolts.
  return VCC_CALIBRATION / result;
}

//========================= END =========================