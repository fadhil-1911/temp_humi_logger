DHT22 Loop Operation Flow

This document describes the complete non-blocking DHT22 reading, retry, display, RTC, voltage-monitoring, and SD-card logging operation.

Operation Flow

START loop()
    │
    ▼
currentMillis = millis()
    │
    ▼
Check whether a new DHT22 reading session should start
    │
    ├── Yes
    │     │
    │     ▼
    │   Initialize reading session
    │
    └── No
          │
          ▼
        Skip initialization
          │
          ▼
Check whether a DHT22 retry attempt is due
    │
    ├── No
    │     │
    │     ▼
    │   Skip retry processing
    │
    └── Yes
          │
          ▼
        Update lastRetryTime
          │
          ▼
        Call dht.readData()
          │
          ▼
        Was the reading successful?
          │
          ├── Yes
          │     │
          │     ▼
          │   Read temperature and humidity
          │
          │   dhtStatus      = true
          │   readOK         = true
          │   retryInProgress = false
          │   resultReady    = true
          │
          └── No
                │
                ▼
              Increment retryCount
                │
                ▼
              Has retryCount reached maxRetry?
                │
                ├── Yes
                │     │
                │     ▼
                │   dhtStatus       = false
                │   readOK          = false
                │   retryInProgress = false
                │   resultReady     = true
                │
                └── No
                      │
                      ▼
                    Keep retryInProgress enabled
                    resultReady remains false
                    The next retry occurs in a future
                    loop cycle after retryInterval
          │
          ▼
Check resultReady
    │
    ├── No
    │     │
    │     ▼
    │   Return to loop()
    │
    └── Yes
          │
          ▼
        resultReady = false
        Reset lastReadTimeDHT22
        Read the RTC
        Validate RTC status
        Format date and time
        Read VCC once
          │
          ▼
        Check readOK
          │
          ├── Yes
          │     │
          │     ▼
          │   Display temperature
          │   Display humidity
          │   Set display status flags
          │   Toggle HEARTBEAT_LED
          │   Print values to Serial
          │
          └── No
                │
                ▼
              Display "Err"
              Clear display status flags
              Print failure message to Serial
          │
          ▼
        Open log.csv on the SD card
          │
          ▼
        Was the file opened successfully?
          │
          ├── Yes
          │     │
          │     ▼
          │   sdStatus = true
          │
          │   Write:
          │   - Date
          │   - Time
          │   - Temperature or NA
          │   - Humidity or NA
          │   - DHT22 status
          │   - RTC status
          │   - SD status
          │   - Display 1 status
          │   - Display 2 status
          │   - VCC voltage
          │
          │   Close log.csv
          │   Print logging status to Serial
          │
          └── No
                │
                ▼
              sdStatus = false
              Print SD-card error to Serial
          │
          ▼
Return to loop()

