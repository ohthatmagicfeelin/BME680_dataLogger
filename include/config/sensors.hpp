#ifndef SENSORS_CONFIG_HPP
#define SENSORS_CONFIG_HPP

#include <Arduino.h>

// Add this before the SensorType enum
enum class SensorId : uint8_t {
    BME680 = 1,
    SOIL_MOISTURE = 2,
    BATTERY = 3,
    NETWORK = 4
};

// Sensor configuration
enum class SensorType {
    BME680,
    SOIL_MOISTURE,
    BATTERY_METRICS,
    NETWORK_METRICS,
    NONE  // Used for empty slots
};

// Maximum number of active sensors
static const int MAX_ACTIVE_SENSORS = 3;

// Active sensor configuration
static const SensorType ACTIVE_SENSORS[MAX_ACTIVE_SENSORS] = {
    SensorType::SOIL_MOISTURE,
    // SensorType::BME680,
    // SensorType::BATTERY_METRICS,
    SensorType::NETWORK_METRICS,
    SensorType::NONE
};

// BME680 configuration
static const int BME_SCL = 22;
static const int BME_SDA = 21;
static const int BME_ADDRESS = 0x77;

// Soil Moisture configuration
static const int SOIL_MOISTURE_PIN = 32;  
static const int SOIL_MOISTURE_POWER_PIN = 27;
static const int SOIL_MOISTURE_AIR_VALUE = 2800;    // Reading in air
static const int SOIL_MOISTURE_WATER_VALUE = 950;   // Reading in water

#endif 