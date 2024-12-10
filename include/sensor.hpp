#ifndef SENSOR_HPP
#define SENSOR_HPP

#include <Adafruit_BME680.h>
#include "types.hpp"
#include "config.hpp"

class SensorManager {
public:
    static bool initialize();
    static SensorData read();
    
private:
    static bool initializeBME680();
    static bool initializeSoilMoisture();
    static SensorData readBME680();
    static SensorData readSoilMoisture();
    
    static Adafruit_BME680 bme;
};

#endif 