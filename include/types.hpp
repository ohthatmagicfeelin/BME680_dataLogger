#ifndef TYPES_HPP
#define TYPES_HPP

#include <time.h>
#include "esp_sleep.h"

// Single data point with type and value
struct DataPoint {
    const char* type;  // Name/type of the reading (e.g., "temperature", "humidity", "soil_moisture")
    float value;
};

// Maximum number of different readings a sensor can have
const int MAX_DATA_POINTS_PER_READING = 10;

// A complete reading with multiple data points and metadata
struct StoredReading {
    DataPoint dataPoints[MAX_DATA_POINTS_PER_READING];
    int numDataPoints;
    time_t timestamp;
    int rssi;
};

// Base sensor data structure
struct SensorData {
    DataPoint dataPoints[MAX_DATA_POINTS_PER_READING];
    int numDataPoints;
};

#endif 