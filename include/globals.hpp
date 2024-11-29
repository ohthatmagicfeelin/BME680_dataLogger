#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "types.hpp"

// Maximum number of readings to store
const int MAX_READINGS = 80; 

// Define the storage structure
struct StoredReadingsBuffer {
    StoredReading readings[MAX_READINGS];
    int count;
};

// Global variables
extern RTC_DATA_ATTR StoredReadingsBuffer storedReadings;
extern RTC_DATA_ATTR bool timeInitialized;
extern RTC_DATA_ATTR time_t lastKnownTime;
extern RTC_DATA_ATTR int bootCount;
extern struct tm timeinfo;  // Add this declaration

#endif 