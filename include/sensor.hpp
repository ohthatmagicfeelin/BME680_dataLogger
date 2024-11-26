#ifndef SENSOR_HPP
#define SENSOR_HPP

#include <Adafruit_BME680.h>
#include "types.hpp"

bool initializeBME680();
SensorData readSensorData();

extern Adafruit_BME680 bme;

#endif 