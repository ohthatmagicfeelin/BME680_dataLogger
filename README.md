# ESP32 Environmental Sensor

## Setup

1. Copy `src/config.h.example` to `src/config.h`
2. Edit `config.h` with your credentials:
   - WiFi SSID and password
   - API endpoint URL
   - Authentication token
   - Device ID
3. Build and flash the project using PlatformIO

## Configuration

The `config.h` file contains sensitive information and is not tracked by git. Make sure to:
- Never commit `config.h` to the repository
- Keep your credentials secure
- Use `config.h.example` as a template for required configuration 