# Developer Guide

Complete guide for developers using the ESP32 Multi-Board Template to build firmware projects.

## Table of Contents

- [Getting Started](#getting-started)
- [Project Architecture](#project-architecture)
- [Core Components](#core-components)
- [API Reference](#api-reference)
- [Build System](#build-system)
- [Development Workflow](#development-workflow)

---

## Getting Started

### Prerequisites

- **Windows:** PowerShell 5.1+
- **Linux/macOS:** Bash
- **Arduino CLI** 1.3.1 or later ([installation guide](https://arduino.github.io/arduino-cli/latest/installation/))

### Installation

1. **Create a new repository from this template:**
   - Go to https://github.com/yourusername/esp32-multiboard-template
   - Click "Use this template" → "Create a new repository"
   - Name your repository and choose visibility
   - Click "Create repository"
   - Clone your new repository:
     ```bash
     git clone https://github.com/yourusername/your-new-repo.git
     cd your-new-repo
     ```

2. **Build firmware:**

   **Windows (PowerShell):**
   ```powershell
   .\build.ps1 esp32_dev        # Build for ESP32 DevKit V1
   .\build.ps1 esp32s3_dev      # Build for ESP32-S3 DevKit
   .\build.ps1 all              # Build all boards
   ```

   **Linux/macOS (Bash):**
   ```bash
   ./build.sh esp32_dev         # Build for ESP32 DevKit V1
   ./build.sh esp32s3_dev       # Build for ESP32-S3 DevKit
   ./build.sh all               # Build all boards
   ```

   The build script automatically:
   - Installs ESP32 core (esp32:esp32) if missing
   - Installs required libraries listed in `libraries.txt` (one per line)
   - Compiles firmware for selected board(s)

3. **Upload firmware:**

   **Windows:**
   ```powershell
   .\upload.ps1 -Board esp32_dev -Port COM7
   ```

   **Linux/macOS:**
   ```bash
   ./upload.sh -b esp32_dev -p /dev/ttyUSB0
   ```

### Alternative: Web Flasher

If you have GitHub Pages enabled, you can flash firmware from your browser:

1. Visit `https://yourusername.github.io/esp32-multiboard-template/`
2. Click "Connect" and select your device
3. Click "Install" to flash the latest firmware

See [WEB_FLASHER_SETUP.md](WEB_FLASHER_SETUP.md) for setup instructions.

---

## Project Architecture

### Build System Design

The template uses a **non-standard Arduino pattern** to share code across multiple ESP32 board variants:

1. **Common code** lives in `common/src/` (`.cpp`, `.h` files organized by feature)
2. **Shared setup()/loop()** is in `common/src/main_sketch.ino.inc` (NOT a `.ino` file!)
3. **Board-specific sketches** are minimal in `boards/{board}/{board}.ino` (~15 lines each)
   - Includes `board_config.h` (hardware constants)
   - Includes `main_sketch.ino.inc` (shared implementation)
4. **Board configs** define hardware constants in `boards/{board}/board_config.h`

### Build Process (Automatic)

The build scripts handle compilation automatically:

1. Build script reads `package.json` to get project metadata
2. Build script **temporarily copies** all `.cpp`, `.h`, and `.ino.inc` files from `common/src/` to each board's sketch directory
3. Build script **injects package.json values** into copied `package_config.h` (name, displayName, displayNameShort)
4. Arduino CLI compiles with custom include paths: `-include board_config.h`
5. Build script **cleans up** copied files after compilation
6. **NEVER manually copy/move files** - let the build scripts handle this

**Note:** The original `common/src/package_config.h` contains default values and is never modified. Only the copied version in the board directory is updated during build.

**Example board sketch** (`boards/esp32_dev/esp32_dev.ino`):
```cpp
// Include board configuration first
#include "board_config.h"

// Include shared implementation
#include <src/main_sketch.ino.inc>
```

### Project Structure

```
esp32-multiboard-template/
├── boards/                          # Board-specific sketches
│   ├── esp32_dev/
│   │   ├── esp32_dev.ino           # Minimal sketch (~20 lines)
│   │   ├── board_config.h          # Hardware-specific constants
│   │   └── board.json              # Board metadata (name, FQBN, board manager URL)
│   └── esp32s3_dev/
│       ├── esp32s3_dev.ino
│       ├── board_config.h
│       └── board.json
│
├── common/                          # Shared library code
│   ├── library.properties          # Arduino library metadata
│   └── src/
│       ├── main_sketch.ino.inc     # Shared setup()/loop()
│       ├── version.h               # Firmware version
│       ├── logging/
│       │   ├── logger.h
│       │   └── logger.cpp          # Unicode box-drawing logger
│       ├── power/
│       │   ├── power_manager.h
│       │   └── power_manager.cpp   # Deep sleep, wake detection, battery
│       ├── config/
│       │   ├── config.h            # DeviceConfig struct
│       │   ├── config_manager.h
│       │   └── config_manager.cpp  # NVS-based configuration
│       ├── wifi/
│       │   ├── wifi_manager.h
│       │   └── wifi_manager.cpp    # WiFi AP/client, channel locking
│       ├── portal/
│       │   ├── config_portal.h
│       │   ├── config_portal.cpp   # Web configuration server
│       │   ├── config_portal_html.h
│       │   └── config_portal_css.h
│       ├── mqtt/
│       │   ├── mqtt_manager.h
│       │   └── mqtt_manager.cpp    # Home Assistant auto-discovery
│       ├── ota/
│       │   ├── ota_manager.h
│       │   └── ota_manager.cpp     # OTA firmware updates
│       └── modes/
│           ├── ap_mode_controller.h
│           └── ap_mode_controller.cpp
│
├── build/                           # Build output (generated)
│   ├── esp32_dev/
│   └── esp32s3_dev/
│
├── flasher/                         # Web flasher (deployed to GitHub Pages)
│   ├── index.html                  # ESP Web Tools flasher UI
│   ├── app.js                      # Device selection logic
│   ├── styles.css                  # Flasher styling
│   ├── manifest_esp32_dev.json     # Flash instructions (ESP32)
│   └── manifest_esp32s3_dev.json   # Flash instructions (ESP32-S3)
│
├── scripts/                         # Build and deployment automation
│   ├── generate_manifests.sh       # Generate ESP Web Tools manifests
│   └── generate_latest_json.sh     # Generate release metadata
│
├── .github/workflows/               # GitHub Actions CI/CD
│   ├── build.yml                   # PR validation (build + version check)
│   ├── release.yml                 # Build and deploy on git tag
│   └── deploy-pages.yml            # Deploy flasher to GitHub Pages
│
├── docs/                            # Documentation
│
├── build.ps1                        # Windows build script
├── build.sh                         # Linux/macOS build script
├── libraries.txt                    # Library dependencies (one per line)
└── README.md                        # Main readme
```

### Include Path Rules

**CRITICAL:** All includes in `common/src/` files MUST use simple names (no relative paths):

```cpp
// ✅ GOOD - Simple include
#include "logger.h"
#include "config_manager.h"
#include "mqtt_manager.h"

// ❌ BAD - Relative paths don't work with build system
#include "../logging/logger.h"
#include "../../config/config_manager.h"
```

The build system flattens all files into the sketch directory during compilation, so relative paths will fail.

---

## Core Components

### 1. Logging System (`common/src/logging/`)

Unicode box-drawing logger with automatic timing:

```cpp
#include "logger.h"

LogBox::begin("My Operation");
LogBox::line("Starting process...");
LogBox::linef("Processing %d items", count);
LogBox::end();  // Automatically shows elapsed time
```

**Key Methods:**
- `LogBox::begin(title)` - Start log section with title
- `LogBox::line(message)` - Log a line
- `LogBox::linef(format, ...)` - Log with printf-style formatting
- `LogBox::end()` - End section, show elapsed time
- `LogBox::message(title, message)` - Single-line log

### 2. Power Management (`common/src/power/`)

Deep sleep, wake detection, battery monitoring:

```cpp
#include "power_manager.h"

PowerManager powerMgr;
powerMgr.begin();

// Check wake reason
WakeupReason reason = powerMgr.getWakeupReason();
if (reason == WAKEUP_TIMER) {
    // Normal wake cycle
} else if (reason == WAKEUP_BUTTON) {
    // User pressed wake button
}

// Read battery (if HAS_BATTERY=true in board_config.h)
float voltage = powerMgr.readBatteryVoltage();

// Configure wake button
powerMgr.configureWakeButton(WAKE_BUTTON_PIN, LOW);

// Enable watchdog timer
powerMgr.enableWatchdog(30);  // 30 second timeout

// Sleep for 1 hour
powerMgr.sleepForSeconds(3600);
```

**Key Methods:**
- `begin()` - Initialize power manager
- `getWakeupReason()` - Returns `WAKEUP_FIRST_BOOT`, `WAKEUP_TIMER`, `WAKEUP_BUTTON`, `WAKEUP_RESET_BUTTON`
- `readBatteryVoltage()` - Read battery voltage (if HAS_BATTERY)
- `configureWakeButton(pin, level)` - Configure wake button
- `enableWatchdog(seconds)` - Enable watchdog timer
- `disableWatchdog()` - Disable watchdog timer
- `sleepForSeconds(seconds)` - Enter deep sleep

### 3. Configuration Management (`common/src/config/`)

NVS-based persistent storage:

```cpp
#include "config_manager.h"

ConfigManager configMgr;
configMgr.begin();

// Check if device is configured
if (configMgr.isConfigured()) {
    String ssid = configMgr.getWiFiSSID();
    String password = configMgr.getWiFiPassword();
}

// Set WiFi credentials
configMgr.setWiFiCredentials("MySSID", "MyPassword");

// Set device name
configMgr.setFriendlyName("my-device");

// Configure MQTT
configMgr.setMQTTConfig("mqtt://192.168.1.10:1883", "user", "pass");

// Configure static IP
configMgr.setStaticIPConfig("192.168.1.100", "192.168.1.1", "255.255.255.0", "8.8.8.8", "8.8.4.4");
configMgr.setUseStaticIP(true);
```

**Key Methods:**
- `begin()` - Initialize NVS
- `isConfigured()` - Check if device configured
- `getWiFiSSID()`, `getWiFiPassword()` - Get WiFi credentials
- `getFriendlyName()` - Get device friendly name
- `getMQTTBroker()`, `getMQTTUsername()`, `getMQTTPassword()` - Get MQTT config
- `setWiFiCredentials(ssid, pass)` - Set WiFi credentials
- `setFriendlyName(name)` - Set friendly name
- `setMQTTConfig(broker, user, pass)` - Set MQTT config
- `setStaticIPConfig(ip, gw, sn, dns1, dns2)` - Set static IP
- `setUseStaticIP(enabled)` - Enable/disable static IP

### 4. WiFi Management (`common/src/wifi/`)

AP mode, client mode, channel locking for fast reconnects:

```cpp
#include "wifi_manager.h"

WiFiManager wifiMgr(&configMgr);

// Start AP mode for configuration
wifiMgr.startAccessPoint();
String apName = wifiMgr.getAPName();  // "esp32-XXXXXX"
String apIP = wifiMgr.getAPIPAddress();  // "192.168.4.1"

// Connect to WiFi (uses stored credentials)
if (wifiMgr.connectToWiFi()) {
    int rssi = wifiMgr.getRSSI();
    String ip = wifiMgr.getLocalIP();
    bool connected = wifiMgr.isConnected();
}

// Link with power manager for channel locking
wifiMgr.setPowerManager(&powerMgr);
```

**Key Methods:**
- `startAccessPoint()` - Start AP mode
- `getAPName()`, `getAPIPAddress()` - Get AP info
- `connectToWiFi()` - Connect using stored credentials
- `connectToWiFi(ssid, pass, &retryCount)` - Connect with specific credentials
- `getRSSI()` - Get WiFi signal strength
- `getLocalIP()` - Get local IP address
- `isConnected()` - Check connection status
- `setPowerManager(powerMgr)` - Link with power manager for channel locking

### 5. Config Portal (`common/src/portal/`)

Web-based configuration interface:

```cpp
#include "config_portal.h"

ConfigPortal portal(&configMgr, &wifiMgr);
portal.begin();  // Start web server on port 80

while (!portal.isConfigReceived()) {
    portal.handleClient();
    delay(10);
}

portal.stop();  // Stop web server
```

**Key Methods:**
- `begin(port)` - Start web server on port (default 80)
- `handleClient()` - Process HTTP requests (call in loop)
- `isConfigReceived()` - Check if config submitted
- `stop()` - Stop web server

**Routes:**
- `/` - Configuration form
- `/save` - Save configuration (POST)
- `/ota` - OTA update page
- `/ota/upload` - OTA file upload (POST)
- `/ota/url` - OTA from URL (POST)

### 6. MQTT Telemetry (`common/src/mqtt/`)

Home Assistant auto-discovery with comprehensive metrics:

```cpp
#include "mqtt_manager.h"

MQTTManager mqttMgr(&configMgr);
mqttMgr.begin();

// Connect to broker
if (mqttMgr.connect()) {
    // Prepare telemetry data
    TelemetryData telemetry;
    telemetry.deviceId = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    telemetry.deviceName = configMgr.getFriendlyName();
    telemetry.modelName = BOARD_MODEL;
    telemetry.wakeReason = powerMgr.getWakeupReason();
    telemetry.batteryVoltage = 4.2;
    telemetry.batteryPercentage = 85;
    telemetry.wifiRSSI = wifiMgr.getRSSI();
    telemetry.wifiBSSID = WiFi.BSSIDstr();
    telemetry.loopTimeTotal = 12.5;
    telemetry.loopTimeWiFi = 3.2;
    telemetry.freeHeap = ESP.getFreeHeap();
    
    // Publish all telemetry (batch)
    mqttMgr.publishAllTelemetry(telemetry);
    
    // Publish Home Assistant discovery
    mqttMgr.publishDiscovery(telemetry);
}
```

**Key Methods:**
- `begin()` - Initialize MQTT manager
- `connect()` - Connect to broker
- `publishAllTelemetry(telemetryData)` - Publish all telemetry (batch)
- `publishDiscovery(telemetryData)` - Publish Home Assistant discovery
- Individual publish methods available (see `mqtt_manager.h`)

### 7. OTA Updates (`common/src/ota/`)

Over-the-air firmware updates via config portal:

**File Upload Method:**
```cpp
#include "ota_manager.h"

OTAManager otaMgr;
otaMgr.begin();

// Handled automatically by config portal at /ota route
// User uploads .bin file via web interface
```

**HTTP URL Method:**
```cpp
// Update from GitHub Pages or any HTTP server
String firmwareUrl = "https://user.github.io/repo/firmware/v1.0.1/esp32_dev.ino.bin";
bool success = otaMgr.updateFromURL(firmwareUrl);
```

**Key Features:**
- Progress tracking with callbacks
- Watchdog timer management
- Automatic reboot after successful update
- Error handling and rollback protection (ESP32 dual-partition scheme)

**Partition Requirements:**
- Must use `min_spiffs` partition scheme (1.9MB APP with OTA)
- Firmware must be < 1.5MB to fit in partition

---

## API Reference

### TelemetryData Struct

The `TelemetryData` struct supports these metrics for MQTT publishing:

| Field | Type | Description | Skip Value |
|-------|------|-------------|------------|
| `deviceId` | String | Unique device identifier | - |
| `deviceName` | String | Friendly device name | - |
| `modelName` | String | Board model name | - |
| `wakeReason` | WakeupReason | Wake reason (timer/button/reset/boot) | - |
| `batteryVoltage` | float | Battery voltage in volts | 0.0 |
| `batteryPercentage` | int | Battery % (0-100) | -1 |
| `wifiRSSI` | int | WiFi signal strength (dBm) | - |
| `wifiBSSID` | String | WiFi access point MAC | empty |
| `wifiRetryCount` | uint8_t | WiFi connection retries | 255 |
| `loopTimeTotal` | float | Total loop time (seconds) | - |
| `loopTimeWiFi` | float | WiFi connection time | 0.0 |
| `loopTimeWork` | float | Work/processing time | 0.0 |
| `loopTimeOther` | float | Other time | 0.0 |
| `otherRetryCount1` | uint8_t | Generic retry counter 1 | 255 |
| `otherRetryCount2` | uint8_t | Generic retry counter 2 | 255 |
| `lastLogMessage` | String | Last log message | empty |
| `lastLogSeverity` | String | "info", "warning", "error" | - |
| `freeHeap` | uint32_t | Free heap memory (bytes) | 0 |

### Board Configuration Constants

Defined in `boards/{board}/board_config.h`:

```cpp
// Board identification
#define BOARD_NAME "ESP32 DevKit V1"
#define BOARD_MODEL "ESP32"

// Hardware configuration
#define HAS_BATTERY true        // Battery monitoring available?
#define BATTERY_ADC_PIN 35      // ADC pin for battery voltage
#define BATTERY_DIVIDER_R1 100  // Voltage divider R1 (kΩ)
#define BATTERY_DIVIDER_R2 100  // Voltage divider R2 (kΩ)

#define HAS_BUTTON true         // Wake button available?
#define WAKE_BUTTON_PIN 0       // GPIO pin for wake button

#define LED_PIN 2               // Built-in LED pin
```

Use these constants for conditional compilation:

```cpp
#if HAS_BATTERY
  float voltage = powerMgr.readBatteryVoltage();
#endif

#if HAS_BUTTON
  pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);
#endif
```

### WakeupReason Enum

```cpp
enum WakeupReason {
    WAKEUP_FIRST_BOOT,      // First boot after power on
    WAKEUP_TIMER,           // Wake from deep sleep timer
    WAKEUP_BUTTON,          // Wake from button press
    WAKEUP_RESET_BUTTON     // Reset button pressed
};
```

---

## Build System

### Dependencies

**Automatically installed by build script:**
- ESP32 Arduino Core 3.3.2+
- Libraries listed in `libraries.txt` (one library name per line)

**Managing libraries:**

The project uses a centralized `libraries.txt` file in the repository root to manage dependencies. This file lists all required Arduino libraries (one per line) and supports `#` for comments.

**Current libraries (default):**
```
PubSubClient  # MQTT client library
```

**Adding libraries:**

Edit `libraries.txt` and add library names:
```
PubSubClient
ArduinoJson
Adafruit_Sensor
# Add more libraries as needed
```

The build script (`build.ps1` or `build.sh`) automatically installs all listed libraries before compilation. No manual `arduino-cli lib install` commands needed!

### Build Configuration

**Partition Scheme:** `min_spiffs` (1.9MB APP with OTA / 190KB SPIFFS)

This is required for OTA updates to work. Firmware size should be < 1.5MB per board.

### Common Build Issues

**Issue: "Include file not found" error**
- **Cause:** Using relative paths in includes
- **Solution:** Change `#include "../logging/logger.h"` to `#include "logger.h"`

**Issue: "PubSubClient.h not found"**
- **Cause:** Library not installed or missing from `libraries.txt`
- **Solution:** Build script should auto-install. Check `libraries.txt` has the library listed.

**Issue: ".cpp files not compiling"**
- **Cause:** Build script not copying files correctly
- **Solution:** Verify build script copies `.cpp`, `.h`, AND `.ino.inc` files recursively

---

## Development Workflow

### Customizing Application Logic

**Where to add your code:**

Open `common/src/main_sketch.ino.inc` and find the `performCustomWork()` function:

```cpp
void performCustomWork() {
  // START HERE - Replace this simulated work with your actual logic:
  // - Sensor reading (temperature, humidity, GPS, etc.)
  // - Display updates (e-ink, LCD, OLED)
  // - Actuator control (relays, motors, servos)
  // - Data processing or calculations
  
  // Example: Read sensor
  float temperature = readTemperatureSensor();
  
  // Example: Update display
  display.clearDisplay();
  display.println("Temp: " + String(temperature) + "°C");
  display.display();
  
  // Example: Log to serial
  LogBox::messagef("Sensor", "Temperature: %.2f°C", temperature);
}
```

**What NOT to modify:**
- Helper functions at the bottom (unless you understand the flow)
- `setup()` function (high-level flow is optimized)
- Component initialization order

### Adding Custom Components

1. Create component directory in `common/src/`:
   ```
   common/src/
   └── my_feature/
       ├── my_feature.h
       └── my_feature.cpp
   ```

2. Use simple includes (no relative paths):
   ```cpp
   // ✅ GOOD
   #include "my_feature.h"
   #include "logger.h"
   
   // ❌ BAD
   #include "../logging/logger.h"
   ```

3. Build script automatically copies all files during compilation.

### Testing

1. **Build all boards:**
   ```bash
   .\build.ps1 all
   ```

2. **Check firmware sizes:**
   - ESP32: Should be < 1.5MB
   - ESP32-S3/C3: Should be < 1.5MB

3. **Verify functionality:**
   - First boot AP mode works
   - Config portal accessible
   - WiFi connection succeeds
   - MQTT telemetry publishes (if configured)
   - Deep sleep works (if using PowerManager)

### Re-entering Config Mode

After deployment, you can re-enter configuration mode by:
- Holding WAKE_BUTTON_PIN during boot
- OR letting WiFi connection fail (enters config mode automatically)

---

## Next Steps

- **[Customization Guide](CUSTOMIZATION.md)** - Adding boards and features
- **[PR Workflow](PR_WORKFLOW.md)** - Pull request validation
- **[CI/CD Flow](CICD_FLOW.md)** - Complete automation pipeline
- **[Web Flasher Setup](WEB_FLASHER_SETUP.md)** - Browser-based flashing
- **[Device User Guide](device/USER_GUIDE.md)** - End-user documentation

---

## Example Projects

See `boards/esp32_dev/esp32_dev.ino` for a complete minimal example.

For a real-world implementation, check the included boards which demonstrate:
- Battery monitoring
- Wake button configuration
- MQTT telemetry publishing
- Deep sleep cycles
