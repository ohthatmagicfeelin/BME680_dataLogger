#ifndef TIME_MANAGER_HPP
#define TIME_MANAGER_HPP

#include "network.hpp"

void initializeTime();
bool isNightTime();

extern time_t lastKnownTime;
extern bool timeInitialized;
extern int bootCount;

#endif 