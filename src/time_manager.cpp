#include "time_manager.hpp"
#include "config.hpp"
#include <time.h>
#include <Arduino.h>
#include <WiFi.h>

extern RTC_DATA_ATTR time_t lastKnownTime;
extern RTC_DATA_ATTR bool timeInitialized;
extern RTC_DATA_ATTR int bootCount;

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
    if(!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return false;
    }
    
    const int currentHour = timeinfo.tm_hour;
    
    Serial.printf("Current time: %02d:%02d:%02d\n", 
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Serial.printf("Night time period: %02d:00 - %02d:00\n", 
                 NIGHT_START_HOUR, NIGHT_END_HOUR);
    
    const bool isLateNight = (currentHour >= NIGHT_START_HOUR);
    const bool isEarlyMorning = (currentHour < NIGHT_END_HOUR);
    const bool isNight = isLateNight || isEarlyMorning;
    
    Serial.printf("Is night time: %s (Late night: %s, Early morning: %s)\n", 
                 isNight ? "Yes" : "No",
                 isLateNight ? "Yes" : "No",
                 isEarlyMorning ? "Yes" : "No");
    
    return isNight;
}