#include "time_manager.hpp"
#include "config.hpp"
#include "network.hpp"
#include <Arduino.h>
#include <WiFi.h>

// Initialize the RTC time state
RTC_DATA_ATTR TimeState timeState = {
    .lastKnownTime = 0,
    .lastSuccessfulSync = 0,
    .isNight = false,
    .wakeCyclesPerNight = 0,
    .nightWakeCyclesCounter = 0,
    .timeInitialized = false
};

bool isStateValid() {
    if (!timeState.timeInitialized) return false;
    
    time_t now;
    time(&now);
    
    const bool validSync = (now - timeState.lastSuccessfulSync) < MAX_TIME_WITHOUT_SYNC;
    const bool validCycles = timeState.wakeCyclesPerNight > 0 && 
                            timeState.nightWakeCyclesCounter <= timeState.wakeCyclesPerNight;
    
    return validSync && validCycles;
}

void calculateNightCycles() {
    // Calculate how many wake cycles will occur during night time
    const int nightDurationSeconds = NIGHT_DURATION * 3600; // hours to seconds
    const int sleepDurationSeconds = SLEEP_TIME / 1000000;  // microseconds to seconds
    
    timeState.wakeCyclesPerNight = nightDurationSeconds / sleepDurationSeconds;
    timeState.nightWakeCyclesCounter = 0;
    
    Serial.printf("Night duration: %d hours (%d seconds)\n", NIGHT_DURATION, nightDurationSeconds);
    Serial.printf("Sleep duration: %d seconds\n", sleepDurationSeconds);
    Serial.printf("Wake cycles per night: %d\n", timeState.wakeCyclesPerNight);
}

bool initializeTime() {
    Serial.println("Initializing time...");
    
    // Try to connect to WiFi and sync time
    int retries = 0;
    const int maxRetries = 5;
    
    while (retries < maxRetries) {
        if (connectToWiFi()) {
            configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
            
            // Wait for time sync
            int syncAttempts = 0;
            while (!getLocalTime(&timeinfo) && syncAttempts < 10) {
                Serial.println("Waiting for time sync...");
                delay(500);
                syncAttempts++;
            }
            
            if (getLocalTime(&timeinfo)) {
                time(&timeState.lastKnownTime);
                timeState.lastSuccessfulSync = timeState.lastKnownTime;
                timeState.timeInitialized = true;
                timeState.nightWakeCyclesCounter = 0;
                
                // Calculate night cycles on first boot
                calculateNightCycles();
                
                // Check if we're starting during night time (shouldn't happen normally)
                timeState.isNight = isNightTime();
                
                Serial.printf("Time initialized: %02d:%02d:%02d\n", 
                            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                
                WiFi.disconnect(true);
                WiFi.mode(WIFI_OFF);
                return true;
            }
        }
        
        retries++;
        Serial.printf("Time sync attempt %d failed\n", retries);
        delay(1000);
    }
    
    // If we get here, initialization failed
    Serial.println("Time initialization failed!");
    return false;
}

void updateTimeAfterSleep() {
    if (timeState.lastKnownTime > 0) {
        timeState.lastKnownTime += (SLEEP_TIME / 1000000);
        struct timeval tv = { .tv_sec = timeState.lastKnownTime };
        settimeofday(&tv, NULL);
        
        if (timeState.isNight) {
            timeState.nightWakeCyclesCounter++;
            Serial.printf("Night cycle %d of %d\n", 
                        timeState.nightWakeCyclesCounter, 
                        timeState.wakeCyclesPerNight);
        }
    }
}

bool isNightTime() {
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return timeState.isNight; // Maintain previous state if time check fails
    }
    
    const int currentHour = timeinfo.tm_hour;
    
    Serial.println("\n=== Time Status ===");
    Serial.printf("Current time: %02d:%02d:%02d\n", 
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Serial.printf("Night period: %02d:00 - %02d:00\n", 
                 NIGHT_START_HOUR, NIGHT_END_HOUR);
    
    const bool isLateNight = (currentHour >= NIGHT_START_HOUR);
    const bool isEarlyMorning = (currentHour < NIGHT_END_HOUR);
    const bool isNight = isLateNight || isEarlyMorning;
    
    // Update night state
    if (isNight && !timeState.isNight) {
        // Transitioning into night
        timeState.isNight = true;
        timeState.nightWakeCyclesCounter = 0;
        calculateNightCycles();
    } else if (!isNight && timeState.isNight) {
        // Transitioning out of night
        timeState.isNight = false;
        timeState.nightWakeCyclesCounter = 0;
    }
    
    Serial.printf("Current hour: %d\n", currentHour);
    Serial.printf("Mode: %s\n", isNight ? "NIGHT TIME - Storing readings" : "DAY TIME - Will send readings");
    Serial.println("==================\n");
    
    return isNight;
}

void handleTimeSync() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    int attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 5) {
        Serial.println("Waiting for time sync...");
        delay(500);
        attempts++;
    }
    
    if (getLocalTime(&timeinfo)) {
        time(&timeState.lastKnownTime);
        timeState.lastSuccessfulSync = timeState.lastKnownTime;
        Serial.printf("Time synced: %02d:%02d:%02d\n", 
                    timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        
        timeState.nightWakeCyclesCounter = 0;
    }
}