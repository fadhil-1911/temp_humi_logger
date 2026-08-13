//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   Version: 1.2.3
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Project: Temp/Huma Logger with RTC + DHT22 + SD + TM1637 (Version: 1.2.1 2025-07-19)
// Date: 2026-08-05
// Fixes/additions:
// Added non-blocking DHT22 retry handling with a maximum of 3 attempts

//======== Include library ========
#include <SmartTM1637.h>  // library TM1637
#include <MyDHT22.h>      // library dht22
#include <Wire.h>         // i2c library
#include <RTClib.h>       // library rtc
#include <SdFat.h>        // library sdfat

//======== Set io pin =========
#define HEARTBEAT_LED 9  // external heartbeat LED
#define DHT_PIN 8        // DHT22
#define CLK_PIN_1 2      // TM1637 1
#define DIO_PIN_1 3      // TM1637 1
#define CLK_PIN_2 4      // TM1637 2
#define DIO_PIN_2 5      // TM1637 2
#define CLK_PIN_3 6      // TM1637 3
#define DIO_PIN_3 7      // TM1637 3
#define CS_PIN 10        // Chip select sd card
#define VERSION "v123"


//======== Instantiation Object ========
// Object instantiation: display 1-3 dari class SmartTM1637
// Constructor arguments: CLK_PIN = pin jam, DIO_PIN = pin data
SmartTM1637 display_1(CLK_PIN_1, DIO_PIN_1);  // Temp (TM1637 dot modul version)
SmartTM1637 display_2(CLK_PIN_2, DIO_PIN_2);  // Humadity (TM1637 dot modul version)
SmartTM1637 display_3(CLK_PIN_3, DIO_PIN_3);  // Clock+Colon (TM1637 colon modul version)

MyDHT22 dht(DHT_PIN);  // Object class DHT22
RTC_DS3231 rtc;        // Object classRTC
SdFat SD;              // Object class SD modul
File logFile;          // Object class file sd SdFat

unsigned long lastReadTimeDHT22 = 0;  // Stores the timestamp of the last DHT22 read

unsigned long lastClockUpdate = 0;
const unsigned long clockUpdateInterval = 250;

unsigned long lastHeartbeat = 0;
bool heartbeatState = false;

//====== Retry read dht22 (non blocking) =====
const int maxRetry = 3;
int retryCount = 0;
unsigned long lastRetryTime = 0;
const unsigned long retryInterval = 2000;
bool readOK = false;
bool retryInProgress = false;
float temp = 0.0, humi = 0.0;
bool dhtStatus = false;
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
  Wire.setClock(100000);  // 100 kHz
  Wire.setWireTimeout(25000, true);

  if (!rtc.begin()) {
    Serial.println("RTC initialization failed");
  }

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
    dhtStatus = false;
    disp1Status = false;  // display 1
    disp2Status = false;  // display 2

    retryInProgress = true;

    // Allow the first reading attempt to run immediately
    lastRetryTime = currentMillis - retryInterval;
  }

  //====================================================
  // 2. Process DHT22 retry attempts
  //====================================================
  if (retryInProgress && currentMillis - lastRetryTime >= retryInterval) {

    lastRetryTime = currentMillis;

    if (dht.readData()) {
      temp = dht.getTemperature();
      humi = dht.getHumidity();

      dhtStatus = true;
      readOK = true;
      retryInProgress = false;
      resultReady = true;

    } else {
      retryCount++;

      Serial.print("DHT22 reading attempt failed: ");
      Serial.print(retryCount);
      Serial.print("/");
      Serial.println(maxRetry);

      if (retryCount >= maxRetry) {
        dhtStatus = false;
        readOK = false;
        retryInProgress = false;
        resultReady = true;
      }
    }
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

    if (Wire.getWireTimeoutFlag()) {
      Serial.println("WARNING: I2C timeout during RTC log read");
      Wire.clearWireTimeoutFlag();
      rtcStatus = false;
    } else {
      rtcStatus = (now.year() >= 2020);
    }

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

      /*
      digitalWrite(
        HEARTBEAT_LED,
        !digitalRead(HEARTBEAT_LED)); 
        */

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

      //==================================================
      // Date
      //==================================================
      if (rtcStatus) {
        logFile.print(dateStr);
      } else {
        logFile.print("NA");
      }

      logFile.print(",");

      //==================================================
      // Time
      //==================================================
      if (rtcStatus) {
        logFile.print(timeStr);
      } else {
        logFile.print("NA");
      }

      logFile.print(",");

      //==================================================
      // Temperature
      //==================================================
      if (dhtStatus) {
        logFile.print(temp, 1);
      } else {
        logFile.print("NA");
      }

      logFile.print(",");

      //==================================================
      // Humidity
      //==================================================
      if (dhtStatus) {
        logFile.print(humi, 1);
      } else {
        logFile.print("NA");
      }

      //==================================================
      // Status flags
      //==================================================
      logFile.print(",");
      logFile.print(dhtStatus ? "1" : "0");

      logFile.print(",");
      logFile.print(rtcStatus ? "1" : "0");

      logFile.print(",");
      logFile.print(sdStatus ? "1" : "0");

      logFile.print(",");
      logFile.print(disp1Status ? "1" : "0");

      logFile.print(",");
      logFile.print(disp2Status ? "1" : "0");

      //==================================================
      // VCC
      //==================================================
      logFile.print(",");
      logFile.println(vccVolts, 2);

      logFile.close();

      //==================================================
      // Serial status
      //==================================================
      if (dhtStatus && rtcStatus) {
        Serial.println("✅ Log saved to SD card");

      } else if (!dhtStatus && !rtcStatus) {
        Serial.println("⚠️ DHT22 and RTC failure logged");

      } else if (!dhtStatus) {
        Serial.println("⚠️ DHT22 failure logged");

      } else {
        Serial.println("⚠️ RTC failure logged");
      }

    } else {
      sdStatus = false;
      Serial.println("❌ Failed to open log.csv");
    }
  }

  //====================================================
  // 4. Continuously update the clock display
  //====================================================
  if (currentMillis - lastClockUpdate >= clockUpdateInterval) {
    lastClockUpdate = currentMillis;

    DateTime clockNow = rtc.now();

    if (Wire.getWireTimeoutFlag()) {
      Serial.println("WARNING: I2C timeout");
      Wire.clearWireTimeoutFlag();
      rtcStatus = false;
    } else {
      rtcStatus = true;

      uint8_t hour = clockNow.hour();
      uint8_t minute = clockNow.minute();

      bool showColon =
        (currentMillis / 500) % 2 == 0;

      display_3.printTime(
        hour,
        minute,
        showColon);
    }
  }


  if (currentMillis - lastHeartbeat >= 1000) {
    lastHeartbeat = currentMillis;

    heartbeatState = !heartbeatState;
    digitalWrite(HEARTBEAT_LED, heartbeatState);
  }
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
