Temp/Humi Logger v1.3.0 Reliability
│
├── Sensor Fault Protection
│   ├── Non-blocking retry state machine
│   ├── Maximum 3 attempts
│   ├── DHT22 read failure detection
│   ├── SHT41 communication failure detection
│   ├── Invalid/stale reading protection
│   └── Automatic sensor recovery on next session
│
├── Communication Protection
│   └── I²C
│       ├── 100 kHz bus speed
│       ├── 25 ms timeout
│       ├── TWI recovery
│       └── Timeout flag detection
│
├── Storage Fault Protection
│   ├── SD logging failure detection
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
├── System Health Monitoring
│   └── D9 heartbeat
│       ├── Toggle every 1 second
│       └── Visual main-loop activity indicator
│
└── Firmware Recovery
    └── 8-second hardware watchdog
        └── Automatic MCU reset if main loop hangs
