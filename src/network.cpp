#include "network.hpp"
#include "config.hpp"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "auth_config.h"

extern std::vector<StoredReading> storedReadings;

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

bool sendDataToAPI(const std::vector<StoredReading>& readings) {
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
        
        for (const auto& reading : readings) {
            char timeStr[30];
            struct tm * timeinfo;
            timeinfo = gmtime(&reading.timestamp);
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
            JsonObject presReading = array.add<JsonObject>();
            presReading["type"] = "pressure";
            presReading["value"] = reading.pressure;
            presReading["deviceId"] = deviceId;
            presReading["timestamp"] = timeStr;
            
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
    if (storedReadings.empty()) {
        Serial.println("No stored readings to send");
        return true;
    }

    bool allSent = true;
    
    if (!connectToWiFi()) {
        Serial.println("Failed to connect to WiFi, will try again next wake cycle");
        return false;
    }

    allSent = sendDataToAPI(storedReadings);

    if (allSent) {
        Serial.println("All stored readings sent successfully");
        storedReadings.clear();
    }

    return allSent;
}