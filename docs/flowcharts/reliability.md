```
Temp/Humi Logger v1.3.0 Reliability
│
├── Sensor Fault Protection
│   ├── Non-blocking sensor state machine
│   ├── DHT22
│   │   ├── 1 attempt per 2-second sampling slot
│   │   ├── Read failure detection
│   │   └── Re-attempt on next sampling slot
│   │
│   ├── SHT41
│   │   ├── Maximum 3 attempts per sampling slot
│   │   ├── 200 ms retry interval
│   │   ├── Communication failure detection
│   │   └── Re-attempt on next sampling slot
│   │
│   └── Invalid / stale data protection
│       └── Failed reading is not logged as valid data
│
├── Communication Protection
│   └── I²C
│       ├── 100 kHz bus speed
│       ├── 25 ms transaction timeout
│       ├── Automatic TWI hardware reset on timeout
│       └── Timeout flag detection
│
├── Storage Fault Protection
│   ├── SD initialization failure detection
│   ├── Log file open failure detection
│   ├── SD_OK status flag
│   ├── Disable logging after SD failure
│   └── Core logger continues operating
│
├── Data Integrity
│   ├── Invalid sensor data → NA
│   ├── Invalid RTC timestamp → NA
│   └── CSV diagnostic status flags
│
├── Power Monitoring
│   ├── Internal VCC measurement
│   └── <4.5 V warning
│
└── System Health Monitoring
└── D9 heartbeat
├── Toggle every 1 second
└── Visual main-loop activity indicator
```
