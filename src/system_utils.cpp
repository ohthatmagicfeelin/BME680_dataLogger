#include "system_utils.hpp"
#include <WiFi.h>
#include "config.hpp"

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