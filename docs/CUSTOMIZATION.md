# Customization Guide

## Adding a New Board

This template makes it easy to add support for new ESP32 variants. Follow these steps:

### 1. Create Board Directory

```bash
mkdir boards/my_new_board
```

### 2. Create Board Configuration Header

Create `boards/my_new_board/board_config.h`:

```cpp
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// Board identification
#define BOARD_NAME "My New Board"
#define BOARD_MODEL "ESP32-C3"  // or "ESP32-S2", "ESP32-S3", etc.

// Hardware configuration
#define HAS_BATTERY true        // Does board have battery monitoring?
#define BATTERY_ADC_PIN 35      // ADC pin for battery voltage (if HAS_BATTERY)
#define BATTERY_DIVIDER_R1 100  // Voltage divider R1 (kΩ)
#define BATTERY_DIVIDER_R2 100  // Voltage divider R2 (kΩ)

#define HAS_BUTTON true         // Does board have wake button?
#define WAKE_BUTTON_PIN 0       // GPIO pin for wake button (if HAS_BUTTON)

#define LED_PIN 2               // Built-in LED pin

// Optional: Add custom constants
#define MY_CUSTOM_PIN 15

#endif // BOARD_CONFIG_H
```

### 3. Create Minimal Sketch

Create `boards/my_new_board/my_new_board.ino`:

```cpp
#include "board_config.h"

void setup() {
    Serial.begin(115200);
    
    // Validate board configuration
    #ifndef BOARD_NAME
    #error "BOARD_NAME not defined in board_config.h"
    #endif
    
    pinMode(LED_PIN, OUTPUT);
    
    // Your setup code here
}

void loop() {
    // Your loop code here
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    delay(1000);
}
```

### 4. Update Build Script

Add your board to the `$boards` hashtable in `build.ps1` (Windows) or `build.sh` (Linux/macOS):

**Windows (build.ps1):**
```powershell
$boards = @{
    'esp32_dev' = @{
        Name = "ESP32 DevKit V1"
        FQBN = "esp32:esp32:esp32"
        Path = "boards/esp32_dev"
    }
    'esp32s3_dev' = @{
        Name = "ESP32-S3 DevKit"
        FQBN = "esp32:esp32:esp32s3"
        Path = "boards/esp32s3_dev"
    }
    'my_new_board' = @{
        Name = "My New Board"
        FQBN = "esp32:esp32:esp32c3"  # See FQBN list below
        Path = "boards/my_new_board"
    }
}
```

**Linux/macOS (build.sh):**
```bash
declare -A boards=(
    ["esp32_dev"]="ESP32 DevKit V1|esp32:esp32:esp32|boards/esp32_dev"
    ["esp32s3_dev"]="ESP32-S3 DevKit|esp32:esp32:esp32s3|boards/esp32s3_dev"
    ["my_new_board"]="My New Board|esp32:esp32:esp32c3|boards/my_new_board"
)
```

### 5. Build and Test

```bash
.\build.ps1 my_new_board
```

## Common ESP32 FQBNs

| Board | FQBN |
|-------|------|
| ESP32 DevKit V1 | `esp32:esp32:esp32` |
| ESP32-S2 | `esp32:esp32:esp32s2` |
| ESP32-S3 DevKit | `esp32:esp32:esp32s3` |
| ESP32-C3 | `esp32:esp32:esp32c3` |
| ESP32-C6 | `esp32:esp32:esp32c6` |

To find the FQBN for your board:
```bash
arduino-cli board listall esp32
```

## Customizing Features

### Disabling Features

To disable optional features, simply don't use them in your sketch. For example:

**Skip MQTT:**
```cpp
// Don't create MQTTManager instance
// Don't call mqtt methods
```

**Skip Power Management:**
```cpp
// Don't create PowerManager instance
// Device stays awake continuously
```

### Adding Custom Components

Add new components to `common/src/`:

```
common/src/
└── my_feature/
    ├── my_feature.h
    └── my_feature.cpp
```

The build script automatically copies all `.cpp` and `.h` files to the sketch directory during compilation.

**Important:** Use simple includes (no relative paths):

```cpp
// ✓ GOOD - Simple include
#include "my_feature.h"
#include "logger.h"

// ✗ BAD - Relative paths don't work with build system
#include "../logging/logger.h"
```

### Board-Specific Code

Use `board_config.h` constants for conditional compilation:

```cpp
#include "board_config.h"

void setup() {
    #if HAS_BATTERY
    float voltage = powerMgr.readBatteryVoltage();
    #endif
    
    #if HAS_BUTTON
    pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);
    #endif
}
```

### Customizing the Config Portal

Edit `common/src/portal/config_portal.cpp` to add/remove form fields:

```cpp
String ConfigPortal::generateConfigPage() {
    // Add custom fields
    html += "<label>My Custom Setting</label>";
    html += "<input type='text' name='customSetting' value='" + customValue + "'>";
    
    // Handle in handleSubmit()
    String customSetting = _server->arg("customSetting");
    // Save to config manager
}
```

### Customizing MQTT Telemetry

Add custom fields to `TelemetryData` struct in `common/src/mqtt/mqtt_manager.h`:

```cpp
struct TelemetryData {
    // Existing fields...
    
    // Add custom fields
    float customMetric1;
    String customMetric2;
    
    TelemetryData() : 
        customMetric1(0.0f),
        customMetric2("") {}
};
```

Then publish in `publishAllTelemetry()` or `publishDiscovery()`.

### Customizing Power Management

Configure sleep behavior in your sketch:

```cpp
PowerManager powerMgr;

// Configure wake button
powerMgr.configureWakeButton(WAKE_BUTTON_PIN, LOW);  // Wake on LOW

// Enable watchdog with custom timeout
powerMgr.enableWatchdog(30);  // 30 second timeout

// Sleep with custom duration
powerMgr.sleepForSeconds(3600);  // 1 hour
```

## Advanced: Multi-Board Shared Code Patterns

### Pattern 1: Board-Specific Initialization

```cpp
void setup() {
    // Common initialization
    Serial.begin(115200);
    configMgr.begin();
    
    // Board-specific initialization
    #if defined(BOARD_MODEL) && BOARD_MODEL == "ESP32-S3"
    // ESP32-S3 specific code
    #elif defined(BOARD_MODEL) && BOARD_MODEL == "ESP32-C3"
    // ESP32-C3 specific code
    #endif
}
```

### Pattern 2: Optional Hardware Features

```cpp
void readSensors() {
    #if HAS_BATTERY
    float voltage = powerMgr.readBatteryVoltage();
    telemetry.batteryVoltage = voltage;
    #endif
    
    #if HAS_TEMPERATURE_SENSOR
    float temp = readTemperature();
    telemetry.temperature = temp;
    #endif
}
```

### Pattern 3: Board Capability Detection

```cpp
void configurePowerSaving() {
    if (HAS_BATTERY) {
        // Enable aggressive power saving
        WiFi.setSleep(true);
        setCpuFrequencyMhz(80);
    } else {
        // Mains powered, optimize for performance
        WiFi.setSleep(false);
        setCpuFrequencyMhz(240);
    }
}
```

## Testing Your Customization

1. **Build all boards:**
   ```bash
   .\build.ps1 all
   ```

2. **Check firmware sizes:**
   - ESP32: Should be < 1.3MB
   - ESP32-S3/C3: Should be < 1.3MB

3. **Verify functionality:**
   - First boot AP mode works
   - Config portal accessible
   - WiFi connection succeeds
   - MQTT telemetry publishes (if configured)
   - Deep sleep works (if using PowerManager)

## Troubleshooting

### Build fails with "Include file not found"

**Issue:** Using relative paths in includes.

**Solution:** Use simple includes without `../`:
```cpp
// Change: #include "../logging/logger.h"
// To:     #include "logger.h"
```

### Board not compiling with undefined reference errors

**Issue:** Missing board configuration constants.

**Solution:** Ensure `board_config.h` defines all required constants. Check `boards/esp32_dev/board_config.h` for reference.

### MQTT not publishing

**Issue:** Library not installed.

**Solution:** Build script should auto-install. If not, manually install:
```bash
arduino-cli lib install PubSubClient
```

### Deep sleep not working

**Issue:** Watchdog timer preventing sleep.

**Solution:** Disable watchdog before sleep:
```cpp
powerMgr.disableWatchdog();
powerMgr.sleepForSeconds(3600);
```

## Need Help?

- Check example boards in `boards/` directory
- Review component headers in `common/src/` for API documentation
- Open an issue on GitHub
