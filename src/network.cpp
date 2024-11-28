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

bool sendDataToAPI(const StoredReading readings[], int count) {
    if (count == 0) return true;
    
    WiFiClientSecure *client = new WiFiClientSecure;
    client->setInsecure();
    client->setTimeout(10000);
    
    HTTPClient http;
    bool success = false;
    
    try {
        http.begin(*client, apiEndpoint);
        http.addHeader("Content-Type", "application/json");
        String authHeader = "Bearer ";
        authHeader += authToken;
        http.addHeader("Authorization", authHeader);
        
        JsonDocument doc;
        JsonArray array = doc.to<JsonArray>();
        
        for (int i = 0; i < count; i++) {
            char timeStr[30];
            struct tm * timeinfo;
            timeinfo = gmtime(&readings[i].timestamp);
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S.000Z", timeinfo);
            
            // Temperature
            JsonObject tempReading = array.add<JsonObject>();
            tempReading["type"] = "temperature";
            tempReading["value"] = readings[i].temperature;
            tempReading["deviceId"] = deviceId;
            tempReading["timestamp"] = timeStr;
            
            // Humidity
            JsonObject humReading = array.add<JsonObject>();
            humReading["type"] = "humidity";
            humReading["value"] = readings[i].humidity;
            humReading["deviceId"] = deviceId;
            humReading["timestamp"] = timeStr;
            
            // Pressure
            JsonObject presReading = array.add<JsonObject>();
            presReading["type"] = "pressure";
            presReading["value"] = readings[i].pressure;
            presReading["deviceId"] = deviceId;
            presReading["timestamp"] = timeStr;
            
            // Air Quality
            JsonObject gasReading = array.add<JsonObject>();
            gasReading["type"] = "air_quality";
            gasReading["value"] = readings[i].gas;
            gasReading["deviceId"] = deviceId;
            gasReading["timestamp"] = timeStr;
        }
        
        String payload;
        serializeJson(doc, payload);
        Serial.println("Sending payload: " + payload);
        
        int httpResponseCode = http.POST(payload);
        
        if (httpResponseCode == 200 || httpResponseCode == 201) {
            Serial.println("Data sent successfully!");
            success = true;
        } else {
            Serial.println("Error on sending POST: " + String(httpResponseCode));
            Serial.println("Response: " + http.getString());
        }
        
        http.end();
    } catch (...) {
        Serial.println("Exception occurred during HTTP request");
    }
    
    delete client;
    return success;
}

bool sendStoredReadings() {
    if (storedReadings.count == 0) {
        Serial.println("No stored readings to send");
        return true;
    }

    Serial.printf("Attempting to send %d stored readings\n", storedReadings.count);
    bool success = false;
    
    if (!connectToWiFi()) {
        Serial.println("Failed to connect to WiFi, will try again next wake cycle");
        return false;
    }

    // Create JSON array for all readings
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();

    for (int i = 0; i < storedReadings.count; i++) {
        const StoredReading& reading = storedReadings.readings[i];
        
        // Convert timestamp to ISO 8601 format
        char timeStr[30];
        struct tm* timeinfo = gmtime(&reading.timestamp);
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S.000Z", timeinfo);
        
        // Temperature
        JsonObject tempReading = array.add<JsonObject>();
        tempReading["type"] = "temperature";
        tempReading["value"] = reading.temperature;
        tempReading["deviceId"] = deviceId;
        tempReading["timestamp"] = timeStr;
        
        // Humidity
        JsonObject humReading = array.add<JsonObject>();
        humReading["type"] = "humidity";
        humReading["value"] = reading.humidity;
        humReading["deviceId"] = deviceId;
        humReading["timestamp"] = timeStr;
        
        // Pressure
        JsonObject pressReading = array.add<JsonObject>();
        pressReading["type"] = "pressure";
        pressReading["value"] = reading.pressure;
        pressReading["deviceId"] = deviceId;
        pressReading["timestamp"] = timeStr;
        
        // Air Quality
        JsonObject gasReading = array.add<JsonObject>();
        gasReading["type"] = "air_quality";
        gasReading["value"] = reading.gas;
        gasReading["deviceId"] = deviceId;
        gasReading["timestamp"] = timeStr;
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