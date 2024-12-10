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
    
    // First boot or invalid state
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
    
    if (!initializeSoilSensor()) {
        Serial.println("Failed to initialize BME680");
        enterErrorState();
        return;
    }
    
    // Read and store sensor data
    SensorData sensorData = readSoilSensorData();
    storeReading(sensorData);

    updateTimeAfterSleep();
    
    // Check night time status after updating time
    timeState.isNight = isNightTime();
    
    // Handle data transmission during day time
    if (!timeState.isNight && storedReadings.count > 0) {
        if (!WiFi.isConnected() && !connectToWiFi()) {
            Serial.println("Failed to connect to WiFi for data transmission");
            goToSleep();
            return;
        }
        
        // Update RSSI after successful WiFi connection
        updateCurrentReadingRSSI();
        
        handleTimeSync();  // WiFi is already connected
        if (sendStoredReadings()) {  // Use existing WiFi connection
            Serial.println("Successfully sent stored readings");
            storedReadings.count = 0;
        }
    }
    
    // Only disconnect WiFi at the end
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    goToSleep();
}

void loop() {
    // Empty - using deep sleep
}

