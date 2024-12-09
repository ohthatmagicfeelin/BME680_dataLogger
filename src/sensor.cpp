#include "sensor.hpp"
#include <Wire.h>

Adafruit_BME680 SensorManager::bme;
bool SensorManager::sensorInitialized[MAX_ACTIVE_SENSORS] = {false};

bool SensorManager::initialize() {
    bool allSuccess = true;
    
    for (int i = 0; i < MAX_ACTIVE_SENSORS; i++) {
        if (ACTIVE_SENSORS[i] == SensorType::NONE) continue;
        
        bool success = false;
        switch (ACTIVE_SENSORS[i]) {
            case SensorType::BME680:
                success = initializeBME680();
                break;
            case SensorType::SOIL_MOISTURE:
                success = initializeSoilMoisture();
                break;
            default:
                continue;
        }
        
        sensorInitialized[i] = success;
        allSuccess &= success;
        
        if (!success) {
            Serial.printf("Failed to initialize sensor %d\n", i);
        }
    }
    
    return allSuccess;
}

SensorData SensorManager::readAll() {
    SensorData combinedData = {{{nullptr, 0}}, 0};
    
    for (int i = 0; i < MAX_ACTIVE_SENSORS; i++) {
        if (!sensorInitialized[i]) continue;
        
        SensorData currentData;
        switch (ACTIVE_SENSORS[i]) {
            case SensorType::BME680:
                currentData = readBME680();
                break;
            case SensorType::SOIL_MOISTURE:
                currentData = readSoilMoisture();
                break;
            default:
                continue;
        }
        
        combineSensorData(combinedData, currentData);
    }
    
    return combinedData;
}

void SensorManager::combineSensorData(SensorData& target, const SensorData& source) {
    for (int i = 0; i < source.numDataPoints; i++) {
        if (target.numDataPoints >= MAX_DATA_POINTS_PER_READING) {
            Serial.println("Warning: Maximum data points reached, some readings ignored");
            break;
        }
        target.dataPoints[target.numDataPoints++] = source.dataPoints[i];
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
    float totalReading = 0.0;
    
    for(int i = 0; i < numReadings; i++) {
        totalReading += analogRead(SOIL_MOISTURE_PIN);
        delay(20);
    }
    
    float rawValue = totalReading / numReadings;
    
    digitalWrite(SOIL_MOISTURE_POWER_PIN, LOW);
    
    float moistureValue = constrain(rawValue, SOIL_MOISTURE_WATER_VALUE, SOIL_MOISTURE_AIR_VALUE);
    float moisturePercent = 100.0f * (moistureValue - SOIL_MOISTURE_AIR_VALUE) / 
                           (SOIL_MOISTURE_WATER_VALUE - SOIL_MOISTURE_AIR_VALUE);
    
    data.dataPoints[data.numDataPoints++] = {"soil_moisture_raw", rawValue};
    data.dataPoints[data.numDataPoints++] = {"soil_moisture_percent", moisturePercent};
    
    Serial.printf("Soil Moisture - Raw: %.2f, Percent: %.2f%%\n", rawValue, moisturePercent);
    
    return data;
} 