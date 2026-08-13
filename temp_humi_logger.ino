//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   Version: 1.3.0
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Project: Temp/Huma Logger with RTC + DHT22 + SD + TM1637 (Version: 1.2.1 2025-07-19)
// Date: 2026-08-?
// Fixes/additions:
// Added SHT41

//======== Include library ========
#include <SmartTM1637.h>  // library TM1637
#include <Wire.h>         // i2c library
#include <RTClib.h>       // library rtc
#include <SdFat.h>        // library sdfat

//#define SENSOR_DHT22  // SET FOR DHT22
#define SENSOR_BME280  // SET FOR BME280

#ifdef SENSOR_DHT22
#include <MyDHT22.h>
#endif

#ifdef SENSOR_BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#endif


//======== Set io pin =========
#define HEARTBEAT_LED 13  // internal led
#define DHT_PIN 8         // DHT22
#define CLK_PIN_1 2       // TM1637 1
#define DIO_PIN_1 3       // TM1637 1
#define CLK_PIN_2 4       // TM1637 2
#define DIO_PIN_2 5       // TM1637 2
#define CLK_PIN_3 6       // TM1637 3
#define DIO_PIN_3 7       // TM1637 3
#define CS_PIN 10         // Chip select sd card
#define VERSION "v130"

/*/======== PINOUT ========
DHT22.   Arduino Uno/Nano
VCC   →  5V
GND   →  GND
DAT   →  D8

------------------------------------------------
BME280 — 6 pin
-----------------
VCC → 3.3V
GND → GND
SCL → A5
SDA → A4
CSB → 3.3V
SDO → 3.3V     ← Address = 0x77 (IF GND > Address = 0x76)


-------------------------------------------------
BME280 — 4 pin Address = 0x76
-----------------
VCC → 3.3V
GND → GND
SCL → A5
SDA → A4
*/

//======== Instantiation Object ========
// Object instantiation: display 1-3 dari class SmartTM1637
// Constructor arguments: CLK_PIN = pin jam, DIO_PIN = pin data
SmartTM1637 display_1(CLK_PIN_1, DIO_PIN_1);  // Temp (TM1637 dot modul version)
SmartTM1637 display_2(CLK_PIN_2, DIO_PIN_2);  // Humadity (TM1637 dot modul version)
SmartTM1637 display_3(CLK_PIN_3, DIO_PIN_3);  // Clock+Colon (TM1637 colon modul version)
RTC_DS3231 rtc;                               // Object classRTC
SdFat SD;                                     // Object class SD modul
File logFile;                                 // Object class file sd SdFat

#ifdef SENSOR_DHT22
MyDHT22 dht(DHT_PIN);
#endif

#ifdef SENSOR_BME280
Adafruit_BME280 bme;
#endif

//unsigned long lastReadTimeDHT22 = 0;
unsigned long lastReadTimeDHT22 = 0;  // Stores the timestamp of the last DHT22 read

//====== Retry read dht22 (non blocking) =====
const int maxRetry = 3;
int retryCount = 0;
unsigned long lastRetryTime = 0;
const unsigned long retryInterval = 2000;
bool readOK = false;
bool retryInProgress = false;
float temp = 0.0, humi = 0.0;
bool sensorStatus = false;
bool rtcStatus = false;
bool sdStatus = false;
bool disp1Status = false;
bool disp2Status = false;
bool resultReady = false;



void setup() {
  Serial.begin(9600);
  display_1.begin(4);
  display_2.begin(4);
  display_3.begin(1);
  Wire.begin();
  Wire.setClock(400000);  // I2C Fast Mode

  rtc.begin();

  display_1.print("init");
  display_2.print(VERSION, false, true, true, false);  // print code version
  pinMode(HEARTBEAT_LED, OUTPUT);                      // loop indicator

  // RTC adjustment (if needed). Uncomment once only.
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  //========== SD Card Setup ==========
  sdStatus = SD.begin(CS_PIN);
  if (!sdStatus) {
    Serial.println("❌ Failed to initialize SD card!");
    display_1.print("SdEr");
  } else {
    Serial.println("✅ SD Card OK");
  }

  //========== Header log.csv ==========
  if (!SD.exists("log.csv")) {
    logFile = SD.open("log.csv", FILE_WRITE);
    if (logFile) {
      logFile.println("Date,Time,Temperature_C,Humidity_PCT,DHT_OK,RTC_OK,SD_OK,DISP1_OK,DISP2_OK,VCC_V");
      logFile.close();
    }
  }

#ifdef SENSOR_DHT22
  Serial.println(F("Sensor: DHT22"));
#endif

#ifdef SENSOR_BME280
  Serial.println(F("Sensor: BME280"));

  if (!bme.begin(0x76)) {
    Serial.println(F("BME280 initialization failed!"));
  } else {
    Serial.println(F("BME280 initialization OK"));
  }
#endif

  delay(3000);
}



void loop() {
  unsigned long currentMillis = millis();

  //====================================================
  // 1. Start a DHT22 reading session every 2 seconds
  //====================================================
  if (!retryInProgress && !resultReady && currentMillis - lastReadTimeDHT22 >= 2000) {

    lastReadTimeDHT22 = currentMillis;

    retryCount = 0;
    readOK = false;
    sensorStatus = false;
    disp1Status = false;  // display 1
    disp2Status = false;  // display 2

    retryInProgress = true;

    // Allow the first reading attempt to run immediately
    lastRetryTime = currentMillis - retryInterval;
  }

  //====================================================
  // 2. Process sensor retry attempts
  //====================================================
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


#ifdef SENSOR_BME280

    temp = bme.readTemperature();
    humi = bme.readHumidity();

    if (!isnan(temp) && !isnan(humi)) {
      sensorStatus = true;
      readOK = true;
      retryInProgress = false;
      resultReady = true;

    } else {
      retryCount++;

      Serial.print(F("BME280 reading attempt failed: "));
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

  //====================================================
  // 3. Process the result after the retry attempts are completed
  //====================================================
  if (resultReady) {
    resultReady = false;

    // Start a new reading session 2 seconds after processing the result
    lastReadTimeDHT22 = currentMillis;

    // Get the current time when the result is ready to be saved
    DateTime now = rtc.now();
    rtcStatus = (now.year() >= 2020);

    char dateStr[11];
    char timeStr[9];

    snprintf(
      dateStr,
      sizeof(dateStr),
      "%04d-%02d-%02d",
      now.year(),
      now.month(),
      now.day());

    snprintf(
      timeStr,
      sizeof(timeStr),
      "%02d:%02d:%02d",
      now.hour(),
      now.minute(),
      now.second());

    // Read the voltage only once
    long vccMillivolts = readVcc();
    float vccVolts = vccMillivolts / 1000.0;

    if (readOK) {
      display_1.print(temp, "C", true);
      display_2.print(humi, "h", true);

      disp1Status = true;
      disp2Status = true;

      digitalWrite(
        HEARTBEAT_LED,
        !digitalRead(HEARTBEAT_LED)); // !-AUTO TOGGLE 

      Serial.print("Temp: ");
      Serial.print(temp, 1);
      Serial.print(" | Humidity: ");
      Serial.print(humi, 1);
      Serial.print(" | VCC: ");
      Serial.print(vccVolts, 2);
      Serial.println(" V");

    } else {
      display_1.print("Err");
      display_2.print("Err");

      disp1Status = false;
      disp2Status = false;

      Serial.println("❌ Failed to read DHT22 after 3 attempts");
    }

    //==================================================
    // Log data to the SD card
    //==================================================
    logFile = SD.open("log.csv", FILE_WRITE);

    if (logFile) {
      sdStatus = true;

      logFile.print(dateStr);
      logFile.print(",");

      logFile.print(timeStr);
      logFile.print(",");

      if (sensorStatus) {
        logFile.print(temp, 1);
      } else {
        logFile.print("NA");
      }

      logFile.print(",");

      if (sensorStatus) {
        logFile.print(humi, 1);
      } else {
        logFile.print("NA");
      }

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

      logFile.print(",");
      logFile.println(vccVolts, 2);

      logFile.close();

      if (sensorStatus) {
        Serial.println("✅ Log saved to SD card");
      } else {
        Serial.println("⚠️ DHT22 failure logged");
      }

    } else {
      sdStatus = false;
      Serial.println("❌ Failed to open log.csv");
    }
  }

  //====================================================
  // 4. Continuously update the clock display
  //====================================================
  DateTime clockNow = rtc.now();

  uint8_t hour = clockNow.hour();
  uint8_t minute = clockNow.minute();

  bool showColon =
    (currentMillis / 500) % 2 == 0; // DOUBLE DOT BLINK

  display_3.printTime(
    hour,
    minute,
    showColon);
}

// Internal voltage monitor
long readVcc() {
  // Select AVcc as ADC reference and measure internal 1.1 V reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

  delay(2);  // Allow the reference and multiplexer to stabilize

  // Perform one dummy conversion
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC)) {
  }

  // Perform the actual conversion
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC)) {
  }

  uint16_t result = ADC;

  if (result == 0) {
    return 0;
  }

  // Calibrated internal-reference constant
  const long VCC_CALIBRATION = 1111800L;

  return VCC_CALIBRATION / result;  // millivolts
}


//==================== END ======================
