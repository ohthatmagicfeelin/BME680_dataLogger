#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "types.hpp"
#include "time_manager.hpp"

// Declare external variables
extern RTC_DATA_ATTR StoredReadingsBuffer storedReadings;
extern RTC_DATA_ATTR int bootCount;
extern struct tm timeinfo;

#endif 