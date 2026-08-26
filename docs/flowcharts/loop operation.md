Temp/Humi Logger v1.3.0
│
├── POWER / MCU
│   └── Arduino Uno / Nano (ATmega328P)
│       ├── 5 V supply
│       ├── Internal VCC monitor
│       │   ├── readVcc()
│       │   └── Warning if VCC < 4.5 V
│       │
│       └── Hardware Watchdog
│           ├── Timeout: 8 seconds
│           ├── wdt_reset() every main-loop cycle
│           └── Automatic MCU reset if firmware hangs
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
│   │   └── Humidity
│   │
│   ├── SHT41
│   │   ├── I²C
│   │   │   ├── SDA → A4
│   │   │   └── SCL → A5
│   │   ├── Temperature
│   │   ├── Humidity
│   │   ├── Initialization status detection
│   │   └── Communication failure detection
│   │
│   └── Common Non-Blocking Read State Machine
│       ├── Start sensor reading session
│       ├── Read attempt
│       ├── Retry on failure
│       │   ├── Maximum: 3 attempts
│       │   └── Retry interval: 2000 ms
│       │
│       └── Result
│           ├── SUCCESS
│           │   ├── temp
│           │   ├── humi
│           │   └── SENSOR_OK = 1
│           │
│           └── FAILED
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
    ├── wdt_reset()
    │
    ├── Sensor session scheduler
    │
    ├── Common sensor retry state machine
    │   ├── DHT22 read path
    │   └── SHT41 read path
    │
    ├── Process completed sensor result
    │   ├── Read RTC timestamp
    │   ├── Check I²C timeout
    │   ├── Measure VCC
    │   ├── Check low VCC
    │   ├── Update temperature display
    │   ├── Update humidity display
    │   └── Write CSV record if SD available
    │
    ├── RTC clock display scheduler
    │   └── Every 250 ms
    │
    └── Heartbeat scheduler
        └── Every 1000 ms
