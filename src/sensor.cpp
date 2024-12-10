#include "sensor.hpp"
#include <Wire.h>

Adafruit_BME680 SensorManager::bme;

bool SensorManager::initialize() {
    switch (ACTIVE_SENSOR) {
        case SensorType::BME680:
            return initializeBME680();
        case SensorType::SOIL_MOISTURE:
            return initializeSoilMoisture();
        default:
            Serial.println("Unknown sensor type");
            return false;
    }
}

SensorData SensorManager::read() {
    switch (ACTIVE_SENSOR) {
        case SensorType::BME680:
            return readBME680();
        case SensorType::SOIL_MOISTURE:
            return readSoilMoisture();
        default:
            Serial.println("Unknown sensor type");
            return SensorData{{{nullptr, 0}}, 0};
    }
}

bool SensorManager::initializeBME680() {
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

bool SensorManager::initializeSoilMoisture() {
    pinMode(SOIL_MOISTURE_PIN, INPUT);
    pinMode(SOIL_MOISTURE_POWER_PIN, OUTPUT);
    return true;
}

SensorData SensorManager::readBME680() {
    SensorData data = {{{nullptr, 0}}, 0};
    
    if (!bme.performReading()) {
        Serial.println("Failed to perform BME680 reading!");
        return data;
    }
    
    data.dataPoints[data.numDataPoints++] = {"temperature", static_cast<float>(bme.temperature)};
    data.dataPoints[data.numDataPoints++] = {"humidity", static_cast<float>(bme.humidity)};
    data.dataPoints[data.numDataPoints++] = {"pressure", static_cast<float>(bme.pressure / 100.0)};
    data.dataPoints[data.numDataPoints++] = {"gas", static_cast<float>(bme.gas_resistance / 1000.0)};
    
    return data;
}

SensorData SensorManager::readSoilMoisture() {
    SensorData data = {{{nullptr, 0}}, 0};
    
    digitalWrite(SOIL_MOISTURE_POWER_PIN, HIGH);
    delay(10);
    
    const int numReadings = 5;
    int totalReading = 0;
    
    for(int i = 0; i < numReadings; i++) {
        totalReading += analogRead(SOIL_MOISTURE_PIN);
        delay(20);
    }
    
    int rawValue = totalReading / numReadings;
    
    digitalWrite(SOIL_MOISTURE_POWER_PIN, LOW);
    
    int moistureValue = constrain(rawValue, SOIL_MOISTURE_WATER_VALUE, SOIL_MOISTURE_AIR_VALUE);
    float moisturePercent = map(moistureValue, SOIL_MOISTURE_AIR_VALUE, SOIL_MOISTURE_WATER_VALUE, 0, 100);
    
    data.dataPoints[data.numDataPoints++] = {"soil_moisture_raw", static_cast<float>(rawValue)};
    data.dataPoints[data.numDataPoints++] = {"soil_moisture_percent", moisturePercent};
    
    Serial.printf("Soil Moisture - Raw: %d, Percent: %.2f%%\n", rawValue, moisturePercent);
    
    return data;
} 