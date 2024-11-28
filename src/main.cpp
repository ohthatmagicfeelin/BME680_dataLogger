#include <Arduino.h>
#include <WiFi.h>
#include "config.hpp"
#include "types.hpp"
#include "globals.hpp"
#include "sensor.hpp"
#include "network.hpp"
#include "time_manager.hpp"
#include "esp_sleep.h"

// Define global variables
RTC_DATA_ATTR StoredReadingsBuffer storedReadings = { .count = 0 };
RTC_DATA_ATTR bool timeInitialized = false;
RTC_DATA_ATTR time_t lastKnownTime = 0;
RTC_DATA_ATTR int bootCount = 0;

void storeReading(const SensorData& data) {
    if (storedReadings.count >= MAX_READINGS) {
        Serial.println("Warning: Storage full, cannot store more readings");
        return;
    }

    time_t now;
    time(&now);
    
    StoredReading reading = {
        data.temperature,
        data.humidity,
        data.pressure,
        data.gas,
        now
    };
    
    storedReadings.readings[storedReadings.count] = reading;
    storedReadings.count++;
    
    Serial.println("\nStored reading #" + String(storedReadings.count));
    Serial.println("-----------------------------------");
    Serial.printf("Temperature: %.2f°C\n", data.temperature);
    Serial.printf("Humidity: %.2f%%\n", data.humidity);
    Serial.printf("Pressure: %.2f hPa\n", data.pressure);
    Serial.printf("Gas: %.2f kOhm\n", data.gas);
    Serial.printf("Total readings stored: %d\n", storedReadings.count);
    Serial.printf("Storage used: %d bytes\n", storedReadings.count * sizeof(StoredReading));
    Serial.println("-----------------------------------\n");
}

void goToSleep() {
    // Update last known time before sleep
    time(&lastKnownTime);
    Serial.println("Going to sleep for " + String(SLEEP_TIME / 1000000) + " seconds");
    Serial.printf("Last known time before sleep: %ld\n", lastKnownTime);
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
    
    // Only connect to WiFi and sync time on first boot or if time is not set
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Time not set - attempting initial sync");
        bool timeSync = false;
        int retries = 0;
        const int maxRetries = 5;
        
        while (!timeSync && retries < maxRetries) {
            if (connectToWiFi()) {
                initializeTime();
                if (getLocalTime(&timeinfo)) {
                    timeSync = true;
                    Serial.printf("Time synchronized: %02d:%02d:%02d\n", 
                                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                    // Disconnect WiFi after getting time
                    WiFi.disconnect(true);
                    WiFi.mode(WIFI_OFF);
                }
            }
            if (!timeSync) {
                retries++;
                Serial.printf("Time sync attempt %d failed, retrying...\n", retries);
                delay(1000);
            }
        }
        
        if (!timeSync) {
            Serial.println("Failed to get initial time sync. Restarting...");
            ESP.restart();
        }
    }
    
    if (!initializeBME680()) {
        Serial.println("Failed to initialize BME680");
        goToSleep();
        return;
    }

    // Update time based on sleep duration
    if (lastKnownTime > 0) {
        lastKnownTime += (SLEEP_TIME / 1000000);
        struct timeval tv = { .tv_sec = lastKnownTime };
        settimeofday(&tv, NULL);
        Serial.printf("Updated time after sleep: %ld\n", lastKnownTime);
    }
    
    // Read sensor data
    SensorData sensorData = readSensorData();
    
    if (getLocalTime(&timeinfo)) {
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        Serial.printf("Taking reading at: %s\n", timeStr);
    }
    
    storeReading(sensorData);
    
    // Only attempt to send data during day time
    if (!isNightTime()) {
        if (storedReadings.count > 0 && connectToWiFi()) {
            if (sendStoredReadings()) {
                Serial.println("Successfully sent stored readings");
            }
        }
    }
    
    goToSleep();
}

void loop() {
    // Empty - using deep sleep
}