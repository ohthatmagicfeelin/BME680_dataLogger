#ifndef TYPES_HPP
#define TYPES_HPP

#include <time.h>
#include "esp_sleep.h"

struct SensorData {
    float temperature;
    float humidity;
    float pressure;
    float gas;
};

struct StoredReading {
    float temperature;
    float humidity;
    float pressure;
    float gas;
    time_t timestamp;
    int rssi;
};

#endif 