#ifndef SYSTEM_UTILS_HPP
#define SYSTEM_UTILS_HPP

#include <Arduino.h>
#include "time_manager.hpp"
#include <WiFi.h>
#include "config.hpp"
#include "esp_sleep.h"

void goToSleep();
void enterErrorState();

#endif