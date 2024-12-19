#include <Arduino.h>
#include <WiFi.h>
#include "config.hpp"
#include "types.hpp"
#include "globals.hpp"
#include "sensor.hpp"
#include "network.hpp"
#include "time_manager.hpp"
#include "system_utils.hpp"
#include "esp_sleep.h"
#include "data_manager.hpp"

// Define global variables
RTC_DATA_ATTR StoredReadingsBuffer storedReadings = { .count = 0 };
RTC_DATA_ATTR int bootCount = 0;
struct tm timeinfo;


void setup() {
    Serial.begin(115200);
    Serial.println("\n\n--- New Session Starting ---");
    delay(1000);
    
    bootCount++;
    
    // First boot or invalid state - always connect
    if (bootCount == 1 || !isStateValid()) {
        if (!connectToWiFi()) {
            enterErrorState();
            return;
        }
        if (!initializeTime()) {
            enterErrorState();
            return;
        }
    }
    
    updateTimeAfterSleep();
    timeState.isNight = isNightTime();
    
    // Connect to WiFi if:
    // 1. It's not night time and we have readings to send, or
    // 2. We have a full buffer that needs to be sent regardless of time
    bool shouldConnect = (!timeState.isNight && storedReadings.count > 0) || 
                        (storedReadings.count >= MAX_READINGS);
    
    if (shouldConnect) {
        if (!WiFi.isConnected() && !connectToWiFi()) {
            Serial.println("Failed to connect to WiFi for data transmission");
            // Continue execution - we'll still take measurements
        } else {
            handleTimeSync();  // Sync time while we have WiFi
        }
    }
    
    // Initialize and read sensors
    if (!SensorManager::initialize()) {
        Serial.println("Failed to initialize sensors");
        enterErrorState();
        return;
    }
    
    SensorData sensorData = SensorManager::readAll();
    storeReading(sensorData);
    
    // If we're connected, try to send the data
    if (WiFi.isConnected() && storedReadings.count > 0) {
        if (sendStoredReadings()) {
            Serial.println("Successfully sent stored readings");
            storedReadings.count = 0;
        }
    }
    
    // Cleanup and sleep
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    goToSleep();
}

void loop() {
    // Empty - using deep sleep
}

