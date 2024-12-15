#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Arduino.h>

// Sensor configuration
enum class SensorType {
    BME680,
    SOIL_MOISTURE,
    DEVICE_METRICS,
    NONE  // Used for empty slots
};

// Maximum number of active sensors
static const int MAX_ACTIVE_SENSORS = 3;

// Active sensor configuration
static const SensorType ACTIVE_SENSORS[MAX_ACTIVE_SENSORS] = {
    // SensorType::SOIL_MOISTURE,
    // SensorType::BME680,
    SensorType::DEVICE_METRICS,
    SensorType::NONE,
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

// Add device metrics configuration
static const int BATTERY_VOLTAGE_PIN = 35;  // ADC1_CHANNEL_7
static const float BATTERY_VOLTAGE_DIVIDER_RATIO = 2.0;  // Depends on your voltage divider
static const float ADC_REFERENCE_VOLTAGE = 3.3;  // ESP32 reference voltage
static const int ADC_RESOLUTION = 4095;  // 12-bit ADC

// Battery type configuration
enum class BatteryType {
    LIPO,
    AA_BATTERIES
};

// Configure which battery type is being used
static const BatteryType BATTERY_TYPE = BatteryType::AA_BATTERIES;

// Battery voltage ranges
// LiPo: 3.3V (empty) to 4.2V (full)
static const float LIPO_MIN_VOLTAGE = 3.3;
static const float LIPO_MAX_VOLTAGE = 4.2;

// 3x AA: 3.0V (empty) to 4.5V (full)
static const float AA_MIN_VOLTAGE = 3.0;
static const float AA_MAX_VOLTAGE = 4.5;

#endif