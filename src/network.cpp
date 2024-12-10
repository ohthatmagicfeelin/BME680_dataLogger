#include "network.hpp"
#include "config.hpp"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "auth_config.h"

extern RTC_DATA_ATTR StoredReadingsBuffer storedReadings;

bool connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi!");
        return true;
    }
    
    Serial.println("\nFailed to connect to WiFi");
    return false;
}


bool sendStoredReadings() {
    if (storedReadings.count == 0) {
        Serial.println("No stored readings to send");
        return true;
    }

    Serial.printf("Attempting to send %d stored readings\n", storedReadings.count);
    bool success = false;
    
    // Create JSON array for all readings
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();

    for (int i = 0; i < storedReadings.count; i++) {
        const StoredReading& reading = storedReadings.readings[i];
        
        // Convert to local time for timestamp
        struct tm timeinfo;
        localtime_r(&reading.timestamp, &timeinfo);
        char timeStr[30];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S.000Z", &timeinfo);
        
        // Add each data point as a separate reading
        for (int j = 0; j < reading.numDataPoints; j++) {
            JsonObject dataPoint = array.add<JsonObject>();
            dataPoint["type"] = reading.dataPoints[j].type;
            dataPoint["value"] = reading.dataPoints[j].value;
            dataPoint["deviceId"] = deviceId;
            dataPoint["timestamp"] = timeStr;
        }
        
        // Add RSSI reading
        JsonObject rssiReading = array.add<JsonObject>();
        rssiReading["type"] = "wifi_rssi";
        rssiReading["value"] = reading.rssi;
        rssiReading["deviceId"] = deviceId;
        rssiReading["timestamp"] = timeStr;
    }
    
    String payload;
    serializeJson(doc, payload);
    Serial.println("Sending payload: " + payload);
    
    HTTPClient http;
    http.begin(apiEndpoint);
    http.addHeader("Content-Type", "application/json");
    
    // Format the authorization header correctly
    String authHeader = String("Bearer ") + authToken;  // Add "Bearer " prefix
    http.addHeader("Authorization", authHeader);
    
    int httpResponseCode = http.POST(payload);
    
    if (httpResponseCode == 200 || httpResponseCode == 201) {
        Serial.println("Data sent successfully!");
        storedReadings.count = 0;  // Clear readings after successful send
        success = true;
    } else {
        Serial.println("Error on sending POST: " + String(httpResponseCode));
        Serial.println("Response: " + http.getString());
        
        // Add more debug info
        Serial.println("Authorization header: " + authHeader);
    }
    
    http.end();
    return success;
}

void updateCurrentReadingRSSI() {
    if (storedReadings.count > 0) {
        int currentRssi = WiFi.isConnected() ? WiFi.RSSI() : -100;
        Serial.printf("Current WiFi RSSI: %d dBm\n", currentRssi);
        storedReadings.readings[storedReadings.count - 1].rssi = currentRssi;
    }
}