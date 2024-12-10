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
RTC_DATA_ATTR int bootCount = 0;
struct tm timeinfo;

void storeReading(const SensorData& data) {
    if (storedReadings.count >= MAX_READINGS) {
        Serial.println("Warning: Storage full, cannot store more readings");
        return;
    }

    StoredReading& reading = storedReadings.readings[storedReadings.count];
    reading.timestamp = timeState.lastKnownTime;
    reading.rssi = -100;  // Default to poor signal
    reading.numDataPoints = 0;
    
    // Copy all data points
    for (int i = 0; i < data.numDataPoints; i++) {
        reading.dataPoints[i] = data.dataPoints[i];
        reading.numDataPoints++;
    }
    
    storedReadings.count++;
    
    // Debug output
    Serial.println("\nStored reading #" + String(storedReadings.count));
    Serial.println("-----------------------------------");
    for (int i = 0; i < reading.numDataPoints; i++) {
        Serial.printf("%s: %.2f\n", reading.dataPoints[i].type, reading.dataPoints[i].value);
    }
    Serial.printf("Total readings stored: %d\n", storedReadings.count);
    Serial.println("-----------------------------------\n");
}

void goToSleep() {
    Serial.println("Going to sleep for " + String(SLEEP_TIME / 1000000) + " seconds");
    Serial.printf("Last known time before sleep: %ld\n", timeState.lastKnownTime);
    Serial.flush();
    
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_sleep_enable_timer_wakeup(SLEEP_TIME);
    esp_deep_sleep_start();
}

void enterErrorState() {
    const int LED_PIN = 2;  // Built-in LED pin for ESP32
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("Entering error state - Please reset device");
    while(true) {
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);
    }
}

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
    
    if (!initializeBME680()) {
        Serial.println("Failed to initialize BME680");
        enterErrorState();
        return;
    }
    
    // Read and store sensor data
    SensorData sensorData = readBME680Data();
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

