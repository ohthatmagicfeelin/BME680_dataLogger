#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <vector>
#include "types.hpp"

bool connectToWiFi();
bool sendDataToAPI(const std::vector<StoredReading>& readings);
bool sendStoredReadings();

extern std::vector<StoredReading> storedReadings;

#endif 