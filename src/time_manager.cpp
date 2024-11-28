#include "time_manager.hpp"
#include "config.hpp"
#include "network.hpp"
#include <time.h>
#include <Arduino.h>
#include <WiFi.h>

extern RTC_DATA_ATTR time_t lastKnownTime;
extern RTC_DATA_ATTR bool timeInitialized;
extern RTC_DATA_ATTR int bootCount;

// Add this global variable to track test start time
RTC_DATA_ATTR static time_t debugStartTime = 0;

void initializeTime() {
    bootCount++;
    
    if (connectToWiFi()) {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        
        int retry = 0;
        const int maxRetries = 10;
        struct tm timeinfo;
        
        while(!getLocalTime(&timeinfo) && retry < maxRetries) {
            Serial.println("Waiting for time sync...");
            delay(500);
            retry++;
        }
        
        if (getLocalTime(&timeinfo)) {
            time(&lastKnownTime);
            Serial.println("Time synchronized from NTP!");
            timeInitialized = true;
        } else {
            Serial.println("Failed to get time from NTP server");
        }
    } else if (lastKnownTime > 0) {
        lastKnownTime += (SLEEP_TIME / 1000000);
        struct timeval tv = { .tv_sec = lastKnownTime };
        settimeofday(&tv, NULL);
        Serial.println("Using stored time + sleep duration");
    }
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        Serial.printf("Boot count: %d\n", bootCount);
        Serial.printf("Current time: %02d:%02d:%02d\n", 
                     timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
        Serial.println("Failed to get local time");
    }
}

bool isNightTime() {
    struct tm timeinfo;
    time_t now;
    time(&now);
    
    if(!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return false;
    }
    
    // Initialize debug start time if needed
    if (DEBUG_MODE && debugStartTime == 0) {
        debugStartTime = now;
        Serial.printf("Debug test started at: %ld\n", debugStartTime);
    }
    
    // In debug mode, check if we're within the test window
    if (DEBUG_MODE && DEBUG_FORCE_NIGHT) {
        time_t elapsed = now - debugStartTime;
        bool isWithinTestWindow = elapsed < DEBUG_TEST_DURATION;
        
        Serial.printf("Debug test elapsed time: %ld seconds\n", elapsed);
        Serial.printf("Remaining test time: %ld seconds\n", 
                     isWithinTestWindow ? DEBUG_TEST_DURATION - elapsed : 0);
        Serial.printf("Is within test window: %s\n", 
                     isWithinTestWindow ? "Yes (Night time)" : "No (Day time)");
        
        return isWithinTestWindow;
    }
    
    // Normal night time logic
    const int currentHour = timeinfo.tm_hour;
    const bool isLateNight = (currentHour >= NIGHT_START_HOUR);
    const bool isEarlyMorning = (currentHour < NIGHT_END_HOUR);
    const bool isNight = isLateNight || isEarlyMorning;
    
    return isNight;
}