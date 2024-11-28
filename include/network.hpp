#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <vector>
#include "types.hpp"
#include "globals.hpp"

bool connectToWiFi();
bool sendDataToAPI(const StoredReading readings[], int count);
bool sendStoredReadings();

#endif 