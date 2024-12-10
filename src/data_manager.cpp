#include "data_manager.hpp"
#include "time_manager.hpp"
#include <Arduino.h>

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