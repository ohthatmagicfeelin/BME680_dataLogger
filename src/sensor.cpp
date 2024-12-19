#include "sensor.hpp"
#include <Wire.h>
#include <WiFi.h>
#include "globals.hpp"

Adafruit_BME680 SensorManager::bme;
bool SensorManager::sensorInitialized[MAX_ACTIVE_SENSORS] = {false};

const char* getBatteryTypeString(BatteryType type) {
    switch(type) {
        case BatteryType::LIPO:
            return "lipo";
        case BatteryType::AA_BATTERIES:
            return "aa_batteries";
        default:
            return "unknown";
    }
}

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
            case SensorType::BATTERY_METRICS:
                success = initializeBatteryMetrics();
                break;
            case SensorType::NETWORK_METRICS:
                success = initializeNetworkMetrics();
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
            case SensorType::BATTERY_METRICS:
                currentData = readBatteryMetrics();
                break;
            case SensorType::NETWORK_METRICS:
                currentData = readNetworkMetrics();
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

bool SensorManager::initializeBatteryMetrics() {
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    analogReadResolution(12);  // Set ADC resolution to 12 bits
    return true;
}

bool SensorManager::initializeNetworkMetrics() {
    return true;  // No initialization needed
}

SensorData SensorManager::readBME680() {
    SensorData data = {{{nullptr, 0}}, 0};
    
    if (!bme.performReading()) {
        Serial.println("Failed to perform BME680 reading!");
        return data;
    }
    
    data.dataPoints[data.numDataPoints++] = {"temperature", static_cast<float>(bme.temperature), static_cast<uint8_t>(SensorId::BME680)};
    data.dataPoints[data.numDataPoints++] = {"humidity", static_cast<float>(bme.humidity), static_cast<uint8_t>(SensorId::BME680)};
    data.dataPoints[data.numDataPoints++] = {"pressure", static_cast<float>(bme.pressure / 100.0), static_cast<uint8_t>(SensorId::BME680)};
    data.dataPoints[data.numDataPoints++] = {"gas", static_cast<float>(bme.gas_resistance / 1000.0), static_cast<uint8_t>(SensorId::BME680)};
    
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
    
    data.dataPoints[data.numDataPoints++] = {"soil_moisture_raw", rawValue, static_cast<uint8_t>(SensorId::SOIL_MOISTURE)};
    data.dataPoints[data.numDataPoints++] = {"soil_moisture_percent", moisturePercent, static_cast<uint8_t>(SensorId::SOIL_MOISTURE)};
    
    Serial.printf("Soil Moisture - Raw: %.2f, Percent: %.2f%%\n", rawValue, moisturePercent);
    
    return data;
}

SensorData SensorManager::readBatteryMetrics() {
    SensorData data = {{{nullptr, 0}}, 0};
    
    // Read battery voltage
    const int numReadings = 10;
    float totalReading = 0.0;
    
    for(int i = 0; i < numReadings; i++) {
        totalReading += analogRead(BATTERY_VOLTAGE_PIN);
        delay(10);
    }
    
    float averageReading = totalReading / numReadings;
    
    // Convert ADC reading to voltage
    float voltageRaw = (averageReading / ADC_RESOLUTION) * ADC_REFERENCE_VOLTAGE;
    float batteryVoltage = voltageRaw * BATTERY_VOLTAGE_DIVIDER_RATIO;
    
    // Get voltage range based on battery type
    float minVoltage, maxVoltage;
    switch(BATTERY_TYPE) {
        case BatteryType::LIPO:
            minVoltage = LIPO_MIN_VOLTAGE;
            maxVoltage = LIPO_MAX_VOLTAGE;
            break;
        case BatteryType::AA_BATTERIES:
            minVoltage = AA_MIN_VOLTAGE;
            maxVoltage = AA_MAX_VOLTAGE;
            break;
    }
    
    // Calculate battery percentage based on configured battery type
    float batteryPercent = constrain(
        ((batteryVoltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0,
        0.0,
        100.0
    );
    
    data.dataPoints[data.numDataPoints++] = {"battery_voltage", batteryVoltage, static_cast<uint8_t>(SensorId::BATTERY)};
    data.dataPoints[data.numDataPoints++] = {"battery_percent", batteryPercent, static_cast<uint8_t>(SensorId::BATTERY)};
    data.dataPoints[data.numDataPoints++] = {"battery_type", (float)static_cast<int>(BATTERY_TYPE) + 1.0, static_cast<uint8_t>(SensorId::BATTERY)};
    
    Serial.printf("Battery Metrics - Voltage: %.2fV (%.1f%%) Type: %d\n", 
                 batteryVoltage, batteryPercent, static_cast<int>(BATTERY_TYPE));
    
    return data;
}

SensorData SensorManager::readNetworkMetrics() {
    SensorData data = {{{nullptr, 0}}, 0};
    
    int rssi = WiFi.isConnected() ? WiFi.RSSI() : -100;
    data.dataPoints[data.numDataPoints++] = {"wifi_rssi", static_cast<float>(rssi), static_cast<uint8_t>(SensorId::NETWORK)};
    data.dataPoints[data.numDataPoints++] = {"stored_readings_count", static_cast<float>(storedReadings.count+1), static_cast<uint8_t>(SensorId::NETWORK)};
    
    Serial.printf("Network Metrics - RSSI: %d dBm, Stored Readings: %d\n", 
                 rssi, storedReadings.count);
    
    return data;
} 