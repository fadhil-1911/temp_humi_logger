```text
Temp/Humi Logger v1.3.0
│
├── POWER / MCU
│   └── Arduino Uno / Nano (ATmega328P)
│       ├── 5 V supply
│       └── Internal VCC monitor
│           ├── readVcc()
│           └── Warning if VCC < 4.5 V
│
├── SENSOR
│   │
│   ├── Compile-Time Sensor Selection
│   │   ├── SENSOR_DHT22
│   │   └── SENSOR_SHT41
│   │
│   ├── DHT22
│   │   ├── DATA → D8
│   │   ├── Temperature
│   │   ├── Humidity
│   │   └── Sampling behavior
│   │       ├── Sampling interval → 2000 ms
│   │       ├── 1 read attempt per sampling slot
│   │       └── Failure → NA → next 2-second slot
│   │
│   ├── SHT41
│   │   ├── I²C
│   │   │   ├── SDA → A4
│   │   │   └── SCL → A5
│   │   ├── Temperature
│   │   ├── Humidity
│   │   ├── Initialization status detection
│   │   ├── Communication failure detection
│   │   └── Retry behavior
│   │       ├── Sampling interval → 2000 ms
│   │       ├── Maximum → 3 attempts per sampling slot
│   │       ├── Retry interval → 200 ms
│   │       └── All attempts fail → NA
│   │
│   └── Sensor-Specific Non-Blocking Read Handling
│       ├── Start 2-second sampling slot
│       │
│       ├── DHT22 selected
│       │   ├── Perform one read attempt
│       │   ├── SUCCESS
│       │   │   ├── temp
│       │   │   ├── humi
│       │   │   └── SENSOR_OK = 1
│       │   │
│       │   └── FAILED
│       │       ├── Temperature = NA
│       │       ├── Humidity = NA
│       │       ├── SENSOR_OK = 0
│       │       └── Wait for next 2-second slot
│       │
│       └── SHT41 selected
│           ├── Attempt #1
│           ├── Failure → wait 200 ms
│           ├── Attempt #2
│           ├── Failure → wait 200 ms
│           ├── Attempt #3
│           │
│           ├── SUCCESS on any attempt
│           │   ├── temp
│           │   ├── humi
│           │   └── SENSOR_OK = 1
│           │
│           └── All attempts FAILED
│               ├── Temperature = NA
│               ├── Humidity = NA
│               ├── SENSOR_OK = 0
│               └── Displays → "Err"
│
├── I²C BUS
│   ├── SDA → A4
│   ├── SCL → A5
│   ├── Bus speed → 100 kHz
│   ├── DS3231 RTC
│   ├── SHT41 (when selected)
│   │
│   └── I²C Protection
│       ├── Timeout → 25 ms
│       ├── Automatic TWI reset
│       └── Timeout flag detection
│
├── RTC / TIME
│   └── DS3231
│       ├── I²C → A4/A5
│       │
│       ├── Timestamp
│       │   ├── Date → YYYY-MM-DD
│       │   ├── Time → HH:MM:SS
│       │   └── RTC failure → NA,NA
│       │
│       └── Clock Display Update
│           ├── Update interval → 250 ms
│           └── Colon blink → 500 ms
│
├── DISPLAYS
│   │
│   ├── TM1637 #1 — Temperature
│   │   ├── CLK → D2
│   │   ├── DIO → D3
│   │   ├── Valid reading → Temperature
│   │   └── Failed reading → "Err"
│   │
│   ├── TM1637 #2 — Humidity
│   │   ├── CLK → D4
│   │   ├── DIO → D5
│   │   ├── Valid reading → Humidity
│   │   └── Failed reading → "Err"
│   │
│   └── TM1637 #3 — Clock
│       ├── CLK → D6
│       ├── DIO → D7
│       ├── HH:MM
│       └── Blinking colon
│
├── SD DATA LOGGER
│   └── MicroSD
│       ├── SPI
│       │   ├── CS   → D10
│       │   ├── MOSI → D11
│       │   ├── MISO → D12
│       │   └── SCK  → D13
│       │
│       ├── log.csv
│       │   ├── One record per completed sampling slot
│       │   ├── Date
│       │   ├── Time
│       │   ├── Temp_C
│       │   ├── Humi_PCT
│       │   ├── SENSOR_OK
│       │   ├── RTC_OK
│       │   ├── SD_OK
│       │   ├── DISP1_OK
│       │   ├── DISP2_OK
│       │   └── VCC_V
│       │
│       └── SD Failure Handling
│           ├── SD.open() failure detected
│           ├── SD_OK = 0
│           ├── SD logging disabled
│           ├── Main logger operation continues
│           └── Reinsert SD + RESET to resume logging
│
├── SYSTEM HEARTBEAT
│   └── External LED → D9
│       ├── Toggle every 1000 ms
│       ├── Blinking → main loop alive
│       └── Stops blinking → firmware potentially blocked
│
└── MAIN LOOP
    │
    ├── Sensor sampling scheduler
    │   └── Fixed 2000 ms sampling interval
    │
    ├── Sensor-specific read handling
    │   ├── DHT22
    │   │   └── 1 attempt per sampling slot
    │   │
    │   └── SHT41
    │       ├── Up to 3 attempts per sampling slot
    │       └── 200 ms between retries
    │
    ├── Process completed sensor result
    │   ├── Read RTC timestamp
    │   ├── Check I²C timeout
    │   ├── Measure VCC
    │   ├── Check low VCC
    │   ├── Update temperature display
    │   ├── Update humidity display
    │   └── Write one CSV record if SD available
    │
    ├── RTC clock display scheduler
    │   └── Every 250 ms
    │
    └── Heartbeat scheduler
        └── Every 1000 ms
```


