#include "sensor.hpp"
#include "config.hpp"
#include <Wire.h>

// Define the BME680 object
Adafruit_BME680 bme;

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

SensorData readBME680Data() {
    SensorData data = {{{nullptr, 0}}, 0};  // Initialize empty structure
    
    if (!bme.performReading()) {
        Serial.println("Failed to perform BME680 reading!");
        return data;
    }
    
    // Add each data point with its type
    data.dataPoints[data.numDataPoints++] = {"temperature", static_cast<float>(bme.temperature)};
    data.dataPoints[data.numDataPoints++] = {"humidity", static_cast<float>(bme.humidity)};
    data.dataPoints[data.numDataPoints++] = {"pressure", static_cast<float>(bme.pressure / 100.0)};
    data.dataPoints[data.numDataPoints++] = {"gas", static_cast<float>(bme.gas_resistance / 1000.0)};
    
    return data;
}

bool initializeSoilSensor() {
    // Mock initialization
    return true;
}

SensorData readSoilSensorData() {
    SensorData data = {{{nullptr, 0}}, 0};
    
    // Mock soil moisture reading (random value between 0-100)
    data.dataPoints[data.numDataPoints++] = {"soil_moisture", static_cast<float>(random(0, 100))};
    
    return data;
}

SensorData readAllSensors() {
    SensorData combinedData = {{{nullptr, 0}}, 0};
    
    // Read BME680
    SensorData bmeData = readBME680Data();
    for (int i = 0; i < bmeData.numDataPoints; i++) {
        combinedData.dataPoints[combinedData.numDataPoints++] = bmeData.dataPoints[i];
    }
    
    // Read soil sensor
    SensorData soilData = readSoilSensorData();
    for (int i = 0; i < soilData.numDataPoints; i++) {
        combinedData.dataPoints[combinedData.numDataPoints++] = soilData.dataPoints[i];
    }
    
    return combinedData;
} 