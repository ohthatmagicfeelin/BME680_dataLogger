#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Arduino.h>

// Sensor configuration
enum class SensorType {
    BME680,
    SOIL_MOISTURE,
    NONE  // Used for empty slots
};

// Maximum number of active sensors
static const int MAX_ACTIVE_SENSORS = 3;

// Active sensor configuration
static const SensorType ACTIVE_SENSORS[MAX_ACTIVE_SENSORS] = {
    SensorType::SOIL_MOISTURE,
    SensorType::BME680,
    SensorType::NONE
};

// Base time unit
static const uint64_t ONE_SECOND = 1000000;

// Sleep configuration
static const uint64_t SLEEP_TIME = 15 * 60 * ONE_SECOND; // 15 minutes
static const int MAX_RETRIES = 3;
static const int RETRY_DELAY = 5000;

// BME680 configuration
static const int BME_SCL = 22;
static const int BME_SDA = 21;
static const int BME_ADDRESS = 0x77;

// Soil Moisture configuration
static const int SOIL_MOISTURE_PIN = 32;  
static const int SOIL_MOISTURE_POWER_PIN = 27;
static const int SOIL_MOISTURE_AIR_VALUE = 3000;    // Reading in air
static const int SOIL_MOISTURE_WATER_VALUE = 1000;   // Reading in water

// Time configuration
static const char* ntpServer = "pool.ntp.org";
static const long gmtOffset_sec = 36000 + 3600;    // AEDT: UTC+11 * 3600
static const int daylightOffset_sec = 0; 

// Night time parameters
static const int NIGHT_START_HOUR = 21;
static const int NIGHT_END_HOUR = 6;

#endif