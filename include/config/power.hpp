#ifndef POWER_CONFIG_HPP
#define POWER_CONFIG_HPP

#include <Arduino.h>

// Battery type configuration
enum class BatteryType {
    LIPO,
    AA_BATTERIES
};

// Configure which battery type is being used
static const BatteryType BATTERY_TYPE = BatteryType::AA_BATTERIES;

// Battery voltage ranges
static const float LIPO_MIN_VOLTAGE = 3.3;
static const float LIPO_MAX_VOLTAGE = 4.2;
static const float AA_MIN_VOLTAGE = 3.0;
static const float AA_MAX_VOLTAGE = 4.5;

// Device metrics configuration
static const int BATTERY_VOLTAGE_PIN = 35;  // ADC1_CHANNEL_7
static const float BATTERY_VOLTAGE_DIVIDER_RATIO = 2.0;
static const float ADC_REFERENCE_VOLTAGE = 3.3;
static const int ADC_RESOLUTION = 4095;

#endif 