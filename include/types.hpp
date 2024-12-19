#ifndef TYPES_HPP
#define TYPES_HPP

#include <time.h>
#include "esp_sleep.h"

// Single data point with type and value
struct DataPoint {
    const char* type;  // Name/type of the reading
    float value;
    uint8_t sensorId; // Use an ID instead of string to save memory
};

// Maximum number of different readings a sensor can have
const int MAX_DATA_POINTS_PER_READING = 10;

// Maximum number of readings that can be stored
const int MAX_READINGS = 24; // Reduced from whatever it was before to save RTC memory

// A complete reading with multiple data points and metadata
struct StoredReading {
    DataPoint dataPoints[MAX_DATA_POINTS_PER_READING];
    int numDataPoints;
    time_t timestamp;
};

// Base sensor data structure
struct SensorData {
    DataPoint dataPoints[MAX_DATA_POINTS_PER_READING];
    int numDataPoints;
};

// Buffer for storing multiple readings
struct StoredReadingsBuffer {
    StoredReading readings[MAX_READINGS];
    int count;
};

#endif 