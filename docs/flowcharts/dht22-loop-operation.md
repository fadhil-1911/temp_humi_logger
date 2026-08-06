Loop Operation Flow

START loop()
    ↓
currentMillis = millis()
    ↓
Check whether a new DHT22 session should start
    ├── Yes → Initialize session
    └── No  → Skip initialization
                    ↓
            Paths merge here
                    ↓
Check whether retry attempt is due
    ├── No → Skip retry processing
    │
    └── Yes
          ↓
       Update lastRetryTime
          ↓
       dht.readData() successful?
          ├── Yes
          │     ↓
          │  Read temperature and humidity
          │  dhtStatus = true
          │  readOK = true
          │  retryInProgress = false
          │  resultReady = true
          │
          └── No
                ↓
             retryCount++
                ↓
             retryCount >= maxRetry?
                ├── Yes
                │     ↓
                │  dhtStatus = false
                │  readOK = false
                │  retryInProgress = false
                │  resultReady = true
                │
                └── No
                      ↓
                   Leave resultReady = false
                    ↓
            All paths merge here
                    ↓
Check resultReady
    ├── No → Return to loop()
    │
    └── Yes
          ↓
       resultReady = false
       Reset lastReadTimeDHT22
       Read RTC
       Format date and time
       Read VCC
          ↓
       Check readOK
          ├── Yes → Display values, set statuses,
          │         toggle heartbeat, print values
          │
          └── No  → Display Err, clear display statuses,
                    print failure
                          ↓
                    Open log.csv
                          ↓
                    File opened?
                       ├── Yes → Write CSV and close file
                       └── No  → sdStatus = false
                          ↓
                    Return to loop()