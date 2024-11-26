#include <Arduino.h>
#include <WiFi.h>
#include "config.hpp"
#include "types.hpp"
#include "sensor.hpp"
#include "network.hpp"
#include "time_manager.hpp"
#include "esp_sleep.h"
#include <vector>

// Global variables
Adafruit_BME680 bme;
RTC_DATA_ATTR std::vector<StoredReading> storedReadings;
RTC_DATA_ATTR bool timeInitialized = false;
RTC_DATA_ATTR time_t lastKnownTime = 0;
RTC_DATA_ATTR int bootCount = 0;

void storeReading(const SensorData& data) {
    struct tm timeinfo;
    time_t now;
    time(&now);
    
    // Print the current time
    char timeStr[30];
    if (localtime_r(&now, &timeinfo)) {
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        Serial.printf("Taking reading at: %s\n", timeStr);
    } else {
        Serial.println("Failed to get local time");
    }
    
    // Check vector capacity before pushing
    if (storedReadings.size() >= 100) {  // Set a reasonable maximum
        Serial.println("Warning: Maximum readings stored. Removing oldest.");
        storedReadings.erase(storedReadings.begin());
    }
    
    StoredReading reading = {
        data.temperature,
        data.humidity,
        data.pressure,
        data.gas,
        now
    };
    
    try {
        storedReadings.push_back(reading);
        
        // Print separator line
        Serial.println("\nStored reading #" + String(storedReadings.size()));
        Serial.println("-----------------------------------");
        Serial.printf("Temperature: %.2f°C\n", data.temperature);
        Serial.printf("Humidity: %.2f%%\n", data.humidity);
        Serial.printf("Pressure: %.2f hPa\n", data.pressure);
        Serial.printf("Gas: %.2f kOhm\n", data.gas);
        Serial.println("-----------------------------------\n");
    } catch (const std::exception& e) {
        Serial.println("Error storing reading: " + String(e.what()));
    }
}

void goToSleep() {
    time(&lastKnownTime);
    Serial.println("Going to sleep for " + String(SLEEP_TIME / 1000000) + " seconds");
    Serial.flush();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_sleep_enable_timer_wakeup(SLEEP_TIME);
    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n--- New Session Starting ---");
    delay(1000);
    
    if (!initializeBME680()) {
        Serial.println("Failed to initialize BME680");
        goToSleep();
        return;
    }

    // Always try to connect and sync time at startup
    if (connectToWiFi()) {
        initializeTime();
    }
    
    // Read sensor data
    SensorData sensorData = readSensorData();
    
    // Check if it's night time
    if (isNightTime()) {
        Serial.println("Night time - storing reading");
        storeReading(sensorData);
    } else {
        // During day, try to send any stored readings first
        if (!storedReadings.empty()) {
            Serial.println("Attempting to send stored readings");
            if (sendStoredReadings()) {
                Serial.println("Successfully sent stored readings");
            }
        }
        
        // Store current reading
        storeReading(sensorData);
        
        // Try to send immediately
        if (connectToWiFi()) {
            std::vector<StoredReading> currentReading = {storedReadings.back()};
            if (!sendDataToAPI(currentReading)) {
                Serial.println("Failed to send current readings");
            } else {
                storedReadings.pop_back();
            }
        }
    }
    
    goToSleep();
}

void loop() {
    // Empty - using deep sleep
}