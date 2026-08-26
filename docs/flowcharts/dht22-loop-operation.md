Temp/Humi Logger v1.2.3
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
│   └── DHT22
│       ├── DATA → D8
│       ├── Temperature
│       ├── Humidity
│       │
│       └── Non-blocking Read State Machine
│           ├── Start reading session
│           ├── Read attempt
│           ├── Retry on failure
│           │   ├── Maximum: 3 attempts
│           │   └── Retry interval: 2000 ms
│           │
│           └── Result
│               ├── SUCCESS
│               │   ├── temp
│               │   ├── humi
│               │   └── DHT_OK = 1
│               │
│               └── FAILED
│                   ├── Temperature = NA
│                   ├── Humidity = NA
│                   └── DHT_OK = 0
│
├── RTC / TIME
│   └── DS3231
│       ├── I²C
│       │   ├── SDA → A4
│       │   ├── SCL → A5
│       │   └── Bus speed → 100 kHz
│       │
│       ├── I²C Protection
│       │   ├── Timeout → 25 ms
│       │   ├── Automatic TWI reset
│       │   └── Timeout flag detection
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
│       └── log.csv
│           ├── Date
│           ├── Time
│           ├── Temperature_C
│           ├── Humidity_PCT
│           ├── DHT_OK
│           ├── RTC_OK
│           ├── SD_OK
│           ├── DISP1_OK
│           ├── DISP2_OK
│           └── VCC_V
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
    ├── DHT22 session scheduler
    │
    ├── DHT22 retry state machine
    │
    ├── Process completed sensor result
    │   ├── Read RTC timestamp
    │   ├── Check I²C timeout
    │   ├── Measure VCC
    │   ├── Check low VCC
    │   ├── Update temperature display
    │   ├── Update humidity display
    │   └── Write CSV record
    │
    ├── RTC clock display scheduler
    │   └── Every 250 ms
    │
    └── Heartbeat scheduler
        └── Every 1000 ms



reliability architecture

Temp/Humi Logger Reliability
│
├── Communication Protection
│   └── I²C timeout + TWI recovery
│
├── Power Monitoring
│   └── VCC measurement + <4.5 V warning
│
└── Firmware Recovery
    └── 8-second hardware watchdog