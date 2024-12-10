#ifndef SENSOR_HPP
#define SENSOR_HPP

#include <Adafruit_BME680.h>
#include "types.hpp"

// BME680 specific functions
bool initializeBME680();
SensorData readBME680Data();

// Mock soil sensor functions
bool initializeSoilSensor();
SensorData readSoilSensorData();

// Combined sensor reading function
SensorData readAllSensors();

extern Adafruit_BME680 bme;

#endif 