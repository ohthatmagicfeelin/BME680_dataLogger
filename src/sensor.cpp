#include "sensor.hpp"
#include "config.hpp"
#include <Wire.h>

extern Adafruit_BME680 bme;

bool initializeBME680() {
    Wire.begin(BME_SDA, BME_SCL);
    
    if (!bme.begin(BME_ADDRESS)) {
        Serial.println("Could not find BME680 sensor!");
        return false;
    }

    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);

    return true;
}

SensorData readSensorData() {
    SensorData data = {0, 0, 0, 0};
    
    if (!bme.performReading()) {
        Serial.println("Failed to perform reading!");
        return data;
    }
    
    data.temperature = bme.temperature;
    data.humidity = bme.humidity;
    data.pressure = bme.pressure / 100.0;
    data.gas = bme.gas_resistance / 1000.0;
    
    return data;
} 