#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <time.h>
#include "esp_system.h"
#include "esp_sleep.h"
#include <vector>
#include "config.h"

// Sleep configuration
const uint64_t ONE_SECOND = 1000000; // 1 microsecond = 10^-6 seconds
const uint64_t SLEEP_TIME = 10  * ONE_SECOND; // 10 minutes in microseconds
const int MAX_RETRIES = 3;
const int RETRY_DELAY = 5000; // 5 seconds between retries

// Add BME680 configuration
Adafruit_BME680 bme;
const int BME_SCL = 22;
const int BME_SDA = 21;
const int BME_ADDRESS = 0x77; // or 0x76 depending on your sensor

// Add time configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 39600;    // AEDT: UTC+11 * 3600
const int daylightOffset_sec = 0;    // Already included in gmtOffset_sec

// Define night time parameters (in 24-hour format)
const int NIGHT_START_HOUR = 21;  // 9 PM
const int NIGHT_END_HOUR = 6;     // 6 AM

// RTC_DATA_ATTR ensures the data persists during deep sleep
struct RTC_DATA_ATTR StoredReading {
  float temperature;
  float humidity;
  float pressure;
  float gas;
  time_t timestamp;
};

RTC_DATA_ATTR std::vector<StoredReading> storedReadings;
RTC_DATA_ATTR bool timeInitialized = false;

bool connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) { // 10 second timeout
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


bool initializeBME680() {
  Wire.begin(BME_SDA, BME_SCL);
  
  if (!bme.begin(BME_ADDRESS)) {
    Serial.println("Could not find BME680 sensor!");
    return false;
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  return true;
}


struct SensorData {
  float temperature;
  float humidity;
  float pressure;
  float gas;
};


SensorData readSensorData() {
  SensorData data = {0, 0, 0, 0};
  
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading!");
    return data;
  }
  
  data.temperature = bme.temperature;
  data.humidity = bme.humidity;
  data.pressure = bme.pressure / 100.0; // Convert to hPa
  data.gas = bme.gas_resistance / 1000.0; // Convert to kOhms
  
  return data;
}


bool sendDataToAPI(const std::vector<StoredReading>& readings) {
  WiFiClientSecure *client = new WiFiClientSecure;
  
  // Explicitly disable SSL certificate verification
  client->setInsecure();
  
  // Add timeout to prevent hanging
  client->setTimeout(10000);  // 10 second timeout
  
  HTTPClient http;
  bool success = false;
  
  // Wrap the HTTP request in a try-catch to handle connection errors
  try {
    http.begin(*client, apiEndpoint);
    http.addHeader("Content-Type", "application/json");
    String authHeader = "Bearer ";
    authHeader += authToken;
    http.addHeader("Authorization", authHeader);
    
    // Build JSON array of readings
    String payload = "[";
    
    for (size_t i = 0; i < readings.size(); i++) {
      const auto& reading = readings[i];
      
      // Convert Unix timestamp to ISO 8601 format
      char timeStr[30];
      struct tm * timeinfo;
      timeinfo = gmtime(&reading.timestamp);
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S.000Z", timeinfo);
      
      // Add temperature reading
      if (i > 0) payload += ",";
      payload += "{\"type\":\"temperature\",";
      payload += "\"value\":" + String(reading.temperature, 2) + ",";
      payload += "\"deviceId\":\"" + String(deviceId) + "\",";
      payload += "\"timestamp\":\"" + String(timeStr) + "\"}";
      
      // Add humidity reading
      payload += ",{\"type\":\"humidity\",";
      payload += "\"value\":" + String(reading.humidity, 2) + ",";
      payload += "\"deviceId\":\"" + String(deviceId) + "\",";
      payload += "\"timestamp\":\"" + String(timeStr) + "\"}";
      
      // Add pressure reading
      payload += ",{\"type\":\"pressure\",";
      payload += "\"value\":" + String(reading.pressure, 2) + ",";
      payload += "\"deviceId\":\"" + String(deviceId) + "\",";
      payload += "\"timestamp\":\"" + String(timeStr) + "\"}";
      
      // Add air quality reading
      payload += ",{\"type\":\"air_quality\",";
      payload += "\"value\":" + String(reading.gas, 2) + ",";
      payload += "\"deviceId\":\"" + String(deviceId) + "\",";
      payload += "\"timestamp\":\"" + String(timeStr) + "\"}";
    }
    
    payload += "]";
    
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

void goToSleep() {
  Serial.println("Going to sleep for " + String(SLEEP_TIME / 1000000) + " seconds");
  Serial.flush();  // Ensure all Serial data is sent before sleep
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_sleep_enable_timer_wakeup(SLEEP_TIME);
  esp_deep_sleep_start();
}

// Function to check if it's night time
bool isNightTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return false;
  }
  
  int currentHour = timeinfo.tm_hour;
  
  // Enhanced debug print
  Serial.printf("Current time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  Serial.printf("Night time period: %02d:00 - %02d:00\n", NIGHT_START_HOUR, NIGHT_END_HOUR);
  
  // Night time is between NIGHT_START_HOUR and NIGHT_END_HOUR
  if (NIGHT_START_HOUR > NIGHT_END_HOUR) {
    // Night spans across midnight
    bool isNight = (currentHour >= NIGHT_START_HOUR || currentHour < NIGHT_END_HOUR);
    Serial.printf("Is night time: %s\n", isNight ? "Yes" : "No");
    return isNight;
  } else {
    // Night is within same day
    bool isNight = (currentHour >= NIGHT_START_HOUR && currentHour < NIGHT_END_HOUR);
    Serial.printf("Is night time: %s\n", isNight ? "Yes" : "No");
    return isNight;
  }
}

// Function to initialize time
void initializeTime() {
  if (!timeInitialized) {
    struct tm timeinfo;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    // Wait for time to be set
    int retry = 0;
    const int retryLimit = 10;
    Serial.println("Waiting for NTP time sync");
    
    while (!getLocalTime(&timeinfo) && retry < retryLimit) {
      Serial.print(".");
      delay(1000);
      retry++;
    }
    
    if (retry < retryLimit) {
      Serial.println("\nTime synchronized!");
      timeInitialized = true;
    } else {
      Serial.println("\nFailed to sync time");
    }
  }
}

// Function to store reading
void storeReading(const SensorData& data) {
  struct tm timeinfo;
  time_t now;
  time(&now);
  
  // Print the current time
  char timeStr[30];
  localtime_r(&now, &timeinfo);
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.printf("Taking reading at: %s\n\n", timeStr);

  
  
  StoredReading reading = {
    data.temperature,
    data.humidity,
    data.pressure,
    data.gas,
    now
  };
  
  storedReadings.push_back(reading);
  Serial.printf("Stored reading #%d\n", storedReadings.size());
  
  // Print the sensor values
  Serial.printf("-----------------------------------\n");
  Serial.printf("Temperature: %.2f°C\n", data.temperature);
  Serial.printf("Humidity: %.2f%%\n", data.humidity);
  Serial.printf("Pressure: %.2f hPa\n", data.pressure);
  Serial.printf("Gas: %.2f kOhm\n", data.gas);
  Serial.printf("-----------------------------------\n\n");
}

// Function to send stored readings
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

  // Send all readings in one batch
  allSent = sendDataToAPI(storedReadings);

  if (allSent) {
    Serial.println("All stored readings sent successfully");
    storedReadings.clear();
  }

  return allSent;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  if (!initializeBME680()) {
    Serial.println("Failed to initialize BME680");
    goToSleep();
    return;
  }

  // Always try to connect and sync time at startup
  if (connectToWiFi()) {
    initializeTime();
  }
  
  SensorData sensorData = readSensorData();
  
  if (isNightTime()) {
    Serial.println("Night time - storing reading");
    storeReading(sensorData);
    goToSleep();
    return;
  }
  
  // Morning time - try to send stored readings first
  bool storedDataSent = false;
  int retryCount = 0;
  
  while (!storedDataSent && retryCount < MAX_RETRIES) {
    if (retryCount > 0) {
      Serial.println("Retry attempt " + String(retryCount));
      delay(RETRY_DELAY);
    }
    
    storedDataSent = sendStoredReadings();
    retryCount++;
  }
  
  // Store current reading
  storeReading(sensorData);
  
  // Send current reading
  if (connectToWiFi()) {
    std::vector<StoredReading> currentReading = {storedReadings.back()};
    if (!sendDataToAPI(currentReading)) {
      Serial.println("Failed to send current readings");
    }
  }
  
  goToSleep();
}

void loop() {
  // Nothing here - we're using deep sleep
}