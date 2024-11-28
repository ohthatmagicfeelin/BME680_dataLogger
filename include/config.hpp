#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Arduino.h>

// Base time unit
static const uint64_t ONE_SECOND = 1000000;

// Debug configuration
static const bool DEBUG_MODE = false;
static const uint64_t DEBUG_SLEEP_TIME = 10 * ONE_SECOND;     // 10 seconds
static const uint64_t NORMAL_SLEEP_TIME = 10 * 60 * ONE_SECOND; // 10 minutes

// Sleep configuration
static const uint64_t SLEEP_TIME = DEBUG_MODE ? DEBUG_SLEEP_TIME : NORMAL_SLEEP_TIME;
static const int MAX_RETRIES = 3;
static const int RETRY_DELAY = 5000;

// For testing night time logic - 3 minute test window
static const bool DEBUG_FORCE_NIGHT = true;
static const int DEBUG_TEST_DURATION = 60; // 1 minute in seconds
static const time_t DEBUG_START_TIME = 0;   // Will be set on first boot

// BME680 configuration
static const int BME_SCL = 22;
static const int BME_SDA = 21;
static const int BME_ADDRESS = 0x77;

// Time configuration
static const char* ntpServer = "pool.ntp.org";
static const long gmtOffset_sec = 39600;    // AEDT: UTC+11 * 3600
static const int daylightOffset_sec = 0;

// Night time parameters
static const int NIGHT_START_HOUR = 21;
static const int NIGHT_END_HOUR = 6;

#endif