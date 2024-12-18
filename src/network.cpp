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


bool sendHttpRequest(const String& payload) {
    HTTPClient http;
    http.begin(apiEndpoint);
    http.addHeader("Content-Type", "application/json");
    
    String authHeader = String("Bearer ") + authToken;
    http.addHeader("Authorization", authHeader);
    
    int httpResponseCode = http.POST(payload);
    bool success = false;
    
    if (httpResponseCode == 200 || httpResponseCode == 201) {
        Serial.println("Data sent successfully!");
        storedReadings.count = 0;  // Clear readings after successful send
        success = true;
    } else {
        Serial.printf("Error on sending POST: %d\n", httpResponseCode);
        Serial.println("Response: " + http.getString());
        Serial.println("Authorization header: " + authHeader);
    }
    
    http.end();
    return success;
}

const char* getSensorString(uint8_t sensorId) {
    switch(static_cast<SensorId>(sensorId)) {
        case SensorId::BME680: return "bme680";
        case SensorId::SOIL_MOISTURE: return "soil_moisture";
        case SensorId::BATTERY: return "battery";
        case SensorId::NETWORK: return "network";
        default: return "unknown";
    }
}

String createJsonPayload() {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();

    for (int i = 0; i < storedReadings.count; i++) {
        const StoredReading& reading = storedReadings.readings[i];
        
        struct tm timeinfo;
        localtime_r(&reading.timestamp, &timeinfo);
        char timeStr[30];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S.000Z", &timeinfo);
        
        for (int j = 0; j < reading.numDataPoints; j++) {
            JsonObject dataPoint = array.add<JsonObject>();
            dataPoint["type"] = reading.dataPoints[j].type;
            dataPoint["value"] = reading.dataPoints[j].value;
            dataPoint["sensor"] = getSensorString(reading.dataPoints[j].sensorId);
            dataPoint["deviceId"] = deviceId;
            dataPoint["timestamp"] = timeStr;
        }
    }

    String payload;
    serializeJson(doc, payload);
    Serial.println(payload);
    return payload;
}


bool sendStoredReadings() {
    if (storedReadings.count == 0) {
        Serial.println("No stored readings to send");
        return true;
    }

    Serial.printf("Attempting to send %d stored readings\n", storedReadings.count);
    bool success = false;
    int retryCount = 0;
    
    while (!success && retryCount < MAX_RETRIES) {
        if (retryCount > 0) {
            Serial.printf("Retry attempt %d of %d\n", retryCount, MAX_RETRIES);
            delay(RETRY_DELAY);
        }
        
        String payload = createJsonPayload();
        success = sendHttpRequest(payload);
        if (!success) retryCount++;
    }
    
    if (!success) {
        Serial.printf("Failed to send data after %d attempts\n", MAX_RETRIES);
    }
    
    return success;
}




