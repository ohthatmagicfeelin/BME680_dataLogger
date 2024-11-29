#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Arduino.h>

// Base time unit
static const uint64_t ONE_SECOND = 1000000;

// Sleep configuration
static const uint64_t SLEEP_TIME = 10 * 60 * ONE_SECOND; // 10 minutes
static const int MAX_RETRIES = 3;
static const int RETRY_DELAY = 5000;

// BME680 configuration
static const int BME_SCL = 22;
static const int BME_SDA = 21;
static const int BME_ADDRESS = 0x77;

// Time configuration
static const char* ntpServer = "pool.ntp.org";
static const long gmtOffset_sec = 36000 + 3600;    // AEDT: UTC+11 * 3600
static const int daylightOffset_sec = 0; 

// Night time parameters
static const int NIGHT_START_HOUR = 21;
static const int NIGHT_END_HOUR = 6;

#endif