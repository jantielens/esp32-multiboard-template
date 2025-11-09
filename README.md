# ESP32 Multi-Board Template

A production-ready ESP32 firmware template with multi-board support, WiFi management, web-based configuration portal, MQTT telemetry with Home Assistant auto-discovery, and power management.

## Features

- 🔌 **Multi-board architecture** - Easy support for multiple ESP32 variants with shared codebase
- 📡 **WiFi Management** - AP mode for initial setup, client mode with channel locking for fast reconnects
- 🌐 **Web Configuration Portal** - Beautiful web UI for WiFi credentials, static IP, and MQTT settings
- 📊 **MQTT Telemetry** - Home Assistant auto-discovery with comprehensive device metrics
- 🔋 **Power Management** - Deep sleep, wake detection, battery monitoring, watchdog timer
- 💾 **NVS Configuration Storage** - Persistent device configuration
- 📝 **Logging System** - Unicode box-drawing logger with timing
- 🔄 **OTA Updates** - File upload and HTTP URL-based firmware updates via config portal
- 🌐 **Web Flasher** - Browser-based firmware flashing with ESP Web Tools (no drivers needed!)
- 🛠️ **Automated Build System** - PowerShell build script with auto-install of dependencies

## Quick Start

### Prerequisites

- **Windows:** PowerShell 5.1+
- **Linux/macOS:** Bash
- **Arduino CLI** 1.3.1 or later ([installation guide](https://arduino.github.io/arduino-cli/latest/installation/))

### 1. Clone the Template

```bash
git clone https://github.com/yourusername/esp32-multiboard-template.git
cd esp32-multiboard-template
```

### 2. Build Firmware

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
- Installs required libraries (PubSubClient)
- Compiles firmware for selected board(s)

### 3. Upload Firmware

**Windows:**
```powershell
.\upload.ps1 -Board esp32_dev -Port COM7
```

**Linux/macOS:**
Use Arduino CLI directly:
```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 boards/esp32_dev
```

### Alternative: Web Flasher (No Drivers Required!)

**If you have GitHub Pages enabled for this repository**, you can flash firmware directly from your browser:

1. Visit `https://yourusername.github.io/esp32-multiboard-template/`
2. Click "Connect" and select your device
3. Click "Install" to flash the latest firmware

**Setting up the web flasher:**

See [docs/WEB_FLASHER_SETUP.md](docs/WEB_FLASHER_SETUP.md) for complete setup instructions. Quick version:

1. Enable GitHub Pages in repository settings (source: `gh-pages` branch)
2. Create a git tag: `git tag v1.0.0 && git push origin v1.0.0`
3. Wait for GitHub Actions to build and deploy
4. Visit `https://yourusername.github.io/your-repo-name/`

The web flasher works on **any modern browser** (Chrome, Edge, Opera) with Web Serial API support. No USB drivers needed!

### 4. First Boot - WiFi Configuration

On first boot, the device creates an access point:
1. Connect to WiFi network: `esp32-XXXXXX` (where XXXXXX is the last 3 bytes of MAC address)
2. Open browser to: `http://192.168.4.1`
3. Enter WiFi credentials and optional settings (static IP, MQTT broker)
4. Click "Save Configuration"
5. Device reboots and connects to your WiFi network

## Project Structure

```
esp32-multiboard-template/
├── boards/                          # Board-specific sketches
│   ├── esp32_dev/
│   │   ├── esp32_dev.ino           # Minimal sketch (10 lines)
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
│   ├── manifest_esp32s3_dev.json   # Flash instructions (ESP32-S3)
│   └── firmware/                   # Firmware binaries (auto-deployed)
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
│   ├── TEMPLATE_GUIDE.md           # This file
│   ├── CUSTOMIZATION.md            # Adding boards, customizing features
│   ├── WEB_FLASHER_SETUP.md        # Web flasher deployment guide
│   └── COPILOT_INSTRUCTIONS.md     # Agent instructions
│
├── build.ps1                        # Windows build script
├── build.sh                         # Linux/macOS build script
└── README.md                        # Main readme
```

## Build System Architecture

The template uses a **non-standard Arduino pattern** to share code across multiple boards:

1. **Common code** lives in `common/src/` (`.cpp`, `.h` files organized by feature)
2. **Shared setup()/loop()** is in `common/src/main_sketch.ino.inc` (NOT a `.ino` file!)
3. **Board-specific sketches** are minimal in `boards/{board}/{board}.ino` (~20 lines each)
   - Board validation (ensures correct board selected in Arduino IDE)
   - Includes `board_config.h` (hardware constants)
   - Includes `main_sketch.ino.inc` (shared implementation)
4. **Board configs** define hardware constants in `boards/{board}/board_config.h`

**Build process (automatic via scripts):**
- Build script **temporarily copies** all `.cpp` and `.h` files from `common/src/` to each board's sketch directory
- Arduino CLI compiles with custom include paths: `-include board_config.h`
- Build script **cleans up** copied files after compilation
- **NEVER manually copy/move files** - let the build scripts handle this

**Example board sketch** (`boards/esp32_dev/esp32_dev.ino`):
```cpp
// Board validation
#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_LOLIN_D32)
#error "Wrong board selection, please select ESP32 Dev Module"
#endif

// Include board configuration first
#include "board_config.h"

// Include shared implementation
#include <src/main_sketch.ino.inc>
```

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

### 2. Power Management (`common/src/power/`)

Deep sleep, wake detection, battery monitoring:

```cpp
#include "power_manager.h"

PowerManager powerMgr;
powerMgr.begin();

WakeupReason reason = powerMgr.getWakeupReason();
if (reason == WAKEUP_TIMER) {
    // Normal wake cycle
}

// Read battery (if HAS_BATTERY=true in board_config.h)
float voltage = powerMgr.readBatteryVoltage();

// Sleep for 1 hour
powerMgr.sleepForSeconds(3600);
```

### 3. Configuration Management (`common/src/config/`)

NVS-based persistent storage:

```cpp
#include "config_manager.h"

ConfigManager configMgr;
configMgr.begin();

if (configMgr.isConfigured()) {
    String ssid = configMgr.getWiFiSSID();
    String password = configMgr.getWiFiPassword();
}

configMgr.setWiFiCredentials("MySSID", "MyPassword");
configMgr.setFriendlyName("my-device");
configMgr.setMQTTConfig("mqtt://192.168.1.10:1883", "user", "pass");
```

### 4. WiFi Management (`common/src/wifi/`)

AP mode, client mode, channel locking:

```cpp
#include "wifi_manager.h"

WiFiManager wifiMgr(&configMgr);

// Start AP mode for configuration
wifiMgr.startAccessPoint();
String apName = wifiMgr.getAPName();  // "esp32-XXXXXX"

// Connect to WiFi (uses stored credentials)
if (wifiMgr.connectToWiFi()) {
    int rssi = wifiMgr.getRSSI();
    String ip = wifiMgr.getLocalIP();
}
```

### 5. Config Portal (`common/src/portal/`)

Web-based configuration interface:

```cpp
#include "config_portal.h"

ConfigPortal portal(&configMgr, &wifiMgr);
portal.begin();

while (!portal.isConfigReceived()) {
    portal.handleClient();
    delay(10);
}
```

### 6. MQTT Telemetry (`common/src/mqtt/`)

Home Assistant auto-discovery with comprehensive metrics:

```cpp
#include "mqtt_manager.h"

MQTTManager mqttMgr(&configMgr);
mqttMgr.begin();

TelemetryData telemetry;
telemetry.deviceId = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
telemetry.deviceName = configMgr.getFriendlyName();
telemetry.modelName = "ESP32 DevKit V1";
telemetry.wakeReason = powerMgr.getWakeupReason();
telemetry.batteryVoltage = 4.2;
telemetry.batteryPercentage = 85;
telemetry.wifiRSSI = wifiMgr.getRSSI();
telemetry.wifiBSSID = WiFi.BSSIDstr();
telemetry.loopTimeTotal = 12.5;
telemetry.loopTimeWiFi = 3.2;
telemetry.freeHeap = ESP.getFreeHeap();

mqttMgr.publishAllTelemetry(telemetry);
```

### 7. OTA Updates (`common/src/ota/`)

Over-the-air firmware updates via config portal:

**File Upload Method:**
1. Build new firmware: `.\build.ps1 esp32_dev`
2. Connect to device's config portal: `http://<device-ip>/`
3. Navigate to OTA page: `http://<device-ip>/ota`
4. Upload `build/esp32_dev/esp32_dev.ino.bin`

**HTTP URL Method:**
```cpp
#include "ota/ota_manager.h"

OTAManager otaMgr;
otaMgr.begin();

// Update from GitHub Pages or any HTTP server
String firmwareUrl = "https://yourusername.github.io/your-repo/firmware/v1.0.1/esp32_dev.ino.bin";
bool success = otaMgr.updateFromURL(firmwareUrl);
```

The OTA manager handles:
- Progress tracking with callbacks
- Watchdog timer management
- Automatic reboot after successful update
- Error handling and rollback protection (ESP32 dual-partition scheme)

## Available Telemetry Fields

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

## Example Sketch

See `boards/esp32_dev/esp32_dev.ino` for a complete example.

## Adding a New Board

See [CUSTOMIZATION.md](docs/CUSTOMIZATION.md) for detailed instructions on adding new board support.

## Dependencies

**Automatically installed by build script:**
- ESP32 Arduino Core 3.3.2+
- PubSubClient 2.8.0+ (MQTT library)

## License

MIT License - See LICENSE file for details

## Contributing

Contributions welcome! Please open an issue or pull request.
