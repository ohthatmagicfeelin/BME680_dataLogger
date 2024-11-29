#ifndef TIME_MANAGER_HPP
#define TIME_MANAGER_HPP

#include "globals.hpp"
#include "config.hpp"
#include <time.h>

// Time state structure
struct TimeState {
    time_t lastKnownTime;
    time_t lastSuccessfulSync;
    bool isNight;
    int wakeCyclesPerNight;
    int nightWakeCyclesCounter;
    bool timeInitialized;
} extern RTC_DATA_ATTR timeState;

// Constants for time calculations
static const int HOURS_IN_DAY = 24;
static const int NIGHT_DURATION = (HOURS_IN_DAY - NIGHT_START_HOUR) + NIGHT_END_HOUR;
static const time_t MAX_TIME_WITHOUT_SYNC = 24 * 60 * 60; // 24 hours in seconds

// Time management functions
bool initializeTime();
bool isNightTime();
void updateTime();
bool isStateValid();
void calculateNightCycles();
void handleTimeSync();
void updateTimeAfterSleep();

#endif 