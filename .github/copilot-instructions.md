# Copilot Agent Instructions for esp32-multiboard-template

## Project Overview

**esp32-multiboard-template** is a production-ready ESP32 firmware template that provides a reusable foundation for multi-board ESP32 projects. It includes WiFi management, web-based configuration portal, MQTT telemetry with Home Assistant auto-discovery, OTA firmware updates, power management with deep sleep, browser-based web flasher, and complete CI/CD automation via GitHub Actions.

**Key Technologies:**
- **Language:** C++ (Arduino framework)
- **Platform:** ESP32 (all variants: ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6)
- **Build Tool:** Arduino CLI 1.3.1+ via PowerShell (Windows) or Bash (Linux/macOS)
- **Libraries:** PubSubClient (MQTT), HTTPClient, HTTPUpdate (OTA)
- **Web Flasher:** ESP Web Tools (browser-based flashing)
- **CI/CD:** GitHub Actions (build validation, release automation, GitHub Pages deployment)
- **Target Runtime:** ESP32 with optional battery power and deep sleep

**Project Size:** ~30 source files, ~10 documentation files, 3 GitHub Actions workflows

## Critical Build Requirements

### Prerequisites (ALWAYS REQUIRED)

Before building, Arduino CLI MUST be installed:

**On Windows (PowerShell):**
- Arduino CLI should be installed to `C:\Program Files\Arduino CLI\arduino-cli.exe`
- OR available in PATH

**On Linux/macOS (Bash):**
- Install Arduino CLI: https://arduino.github.io/arduino-cli/latest/installation/
- Ensure `arduino-cli` is in PATH

**Required libraries** (auto-installed by build scripts):
- PubSubClient (MQTT)
- HTTPClient (built-in with ESP32 core, for OTA)
- HTTPUpdate (built-in with ESP32 core, for OTA)

### Build Commands (TRUST THESE)

**Build a specific board:**
```powershell
# Windows
.\build.ps1 esp32_dev

# Linux/macOS
./build.sh esp32_dev
```

**Valid board names:** `esp32_dev`, `esp32s3_dev`, or any custom board added to build script

**Build all boards:**
```powershell
# Windows
.\build.ps1 all

# Linux/macOS
./build.sh all
```

### Build System Architecture (CRITICAL)

The build system uses a **non-standard Arduino pattern** to share code across multiple hardware variants:

1. **Common code** lives in `common/src/` organized by feature (`.cpp`, `.h` files)
2. **Shared setup()/loop()** is in `common/src/main_sketch.ino.inc` (NOT a `.ino` file!)
3. **Board-specific sketches** are minimal (~20 lines) in `boards/{board}/{board}.ino`
   - Board validation (ensures correct board selected in Arduino IDE)
   - Includes `board_config.h` (hardware constants)
   - Includes `main_sketch.ino.inc` (shared implementation)
4. **Board configs** define hardware constants in `boards/{board}/board_config.h`

**Build Process (automatic via scripts):**
- Build script temporarily **copies** all `.cpp`, `.h`, AND `.ino.inc` files from `common/src/` (recursively) to each board's sketch directory
- **CRITICAL:** Must copy `.ino.inc` files (like `main_sketch.ino.inc`) - this is easy to forget!
- Board `.ino` files include the copied `main_sketch.ino.inc` with: `#include "main_sketch.ino.inc"` (relative path, NOT `<src/...>`)
- Arduino CLI compiles with custom include paths and partition scheme
- Build script **cleans up** copied files after compilation
- **NEVER manually copy/move files** - let the build scripts handle this

**Common Build Script Mistakes (AVOID THESE):**
1.  Forgetting to copy `.ino.inc` files (only copying `.cpp` and `.h`)
   -  Always use: `find common/src \( -name "*.cpp" -o -name "*.h" -o -name "*.ino.inc" \)` (Bash)
   -  Always use: `Get-ChildItem -Filter "*.ino.inc"` and add to `$allCommonFiles` (PowerShell)

2.  Using wrong include path in board `.ino` files: `#include <src/main_sketch.ino.inc>`
   -  Correct: `#include "main_sketch.ino.inc"` (file is copied to same directory)

3.  Using absolute paths in `main_sketch.ino.inc`: `#include <src/logging/logger.h>`
   -  Correct: `#include "logger.h"` (all files are copied to same directory)
   - **ALL includes in .ino.inc files MUST use relative paths** (just the header filename)

**Key Build Property:**
- Partition scheme: `min_spiffs` (1.9MB APP with OTA / 190KB SPIFFS)
- This is **required** for OTA updates to work
- Firmware size should be < 1.5MB per board

**CRITICAL - Include Paths:**
- All includes in `common/src/` files MUST use simple names: `#include "logger.h"`
- **NEVER** use relative paths: `#include "../logging/logger.h"` (will fail in copied files)
- The build system flattens all files into the sketch directory during compilation

### Build Outputs

Binaries are in `build/{board}/`:
- `{board}.ino.bin` - Main firmware
- `{board}.ino.bootloader.bin` - ESP32 bootloader
- `{board}.ino.partitions.bin` - Partition table

## Project Structure & Key Files

### Repository Root Files

```
build.ps1               # Windows build script
build.sh                # Linux/macOS build script (create as needed)
README.md               # User-facing documentation
```

### Directory Structure

```
boards/{board}/                      # Board-specific sketches
  {board}.ino                        # Minimal entry point (~20 lines: validation + includes)
  board_config.h                     # Hardware constants

common/                              # Shared code library
  library.properties                 # Arduino library metadata
  src/
    main_sketch.ino.inc              # Shared setup() and loop() (~100 lines)
    logging/
      logger.h/cpp                   # Unicode box-drawing logger with timing
    power/
      power_manager.h/cpp            # Deep sleep, wake detection, battery monitoring
    config/
      config.h                       # DeviceConfig struct
      config_manager.h/cpp           # NVS-based configuration storage
    wifi/
      wifi_manager.h/cpp             # WiFi AP/client, channel locking, static IP
    portal/
      config_portal.h/cpp            # Web configuration server
      config_portal_html.h           # HTML templates
      config_portal_css.h            # CSS styles
    mqtt/
      mqtt_manager.h/cpp             # Home Assistant auto-discovery + telemetry
    ota/
      ota_manager.h/cpp              # OTA updates (file upload + HTTP URL)
    modes/
      ap_mode_controller.h/cpp       # AP mode startup coordination

build/{board}/                       # Build output (generated)

flasher/                             # Web flasher (deployed to GitHub Pages)
  index.html                         # ESP Web Tools flasher UI
  app.js                             # Device selection logic
  styles.css                         # Flasher styling
  manifest_{board}.json              # Flash instructions (per board)
  firmware/                          # Firmware binaries (auto-deployed)

.github/workflows/                   # GitHub Actions CI/CD
  build.yml                          # PR validation (build + version check)
  release.yml                        # Release automation (on git tag)
  deploy-pages.yml                   # GitHub Pages deployment

scripts/                             # Build and deployment automation
  generate_manifests.sh              # Generate ESP Web Tools manifests
  generate_latest_json.sh            # Generate release metadata

docs/
  CICD_FLOW.md                       # Complete CI/CD pipeline visualization
  PR_WORKFLOW.md                     # Pull request validation guide
  WEB_FLASHER_SETUP.md               # Web flasher deployment guide
  CUSTOMIZATION.md                   # Adding boards, customizing features
  GH_PAGES_BRANCH.md                 # Branch strategy explanation
  README.md                          # Documentation index
```

### Configuration Files

- **`common/library.properties`**: Arduino library metadata
- **`boards/{board}/board_config.h`**: Board-specific hardware constants
  - `BOARD_NAME`, `BOARD_MODEL`
  - `HAS_BATTERY`, `BATTERY_ADC_PIN`, voltage divider constants
  - `HAS_BUTTON`, `WAKE_BUTTON_PIN`
  - `LED_PIN`

## Customizing the Template

**Where to add your application logic:**

1. **Open `common/src/main_sketch.ino.inc`**
2. **Find `performCustomWork()` function** (top of file, clearly marked with "START HERE")
3. **Replace the simulated work** with your actual logic:
   - Sensor reading (temperature, humidity, GPS, etc.)
   - Display updates (e-ink, LCD, OLED)
   - Actuator control (relays, motors, servos)
   - Data processing or calculations

**Example customization:**
```cpp
void performCustomWork() {
  // Read sensor
  float temperature = readTemperatureSensor();
  
  // Display on screen
  display.clearDisplay();
  display.println("Temp: " + String(temperature) + "�C");
  display.display();
  
  // Log to serial
  LogBox::messagef("Sensor", "Temperature: %.2f�C", temperature);
}
```

**What NOT to modify:**
- Helper functions at the bottom (unless you understand the flow)
- setup() function (high-level flow is optimized)
- Component initialization order

**Re-entering config mode after deployment:**
- Hold WAKE_BUTTON_PIN during boot
- OR let WiFi connection fail (enters config mode automatically)

## Code Change Guidelines

### Adding New Features

1. **Add code to `common/src/{feature}/`** (organized by feature area)
2. **Use board-specific constants** from `board_config.h`:
   ```cpp
   #include "board_config.h"
   
   void myFunction() {
     #if HAS_BATTERY
     float voltage = powerMgr.readBatteryVoltage();
     #endif
     
     #if HAS_BUTTON
     pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);
     #endif
   }
   ```

3. **Use simple includes** (no relative paths):
   ```cpp
   // GOOD
   #include "logger.h"
   #include "config_manager.h"
   
   // BAD - will fail when copied
   #include "../logging/logger.h"
   ```

4. **Test on all boards**: `./build.ps1 all`

### Adding a New Board

See `docs/CUSTOMIZATION.md` for complete guide. Summary:

1. Create `boards/newboard/` directory
2. Add `newboard.ino` (copy from existing board, change validation)
3. Add `board_config.h` with hardware constants
4. Update `$boards` hashtable in `build.ps1` and/or `build.sh`
5. Test build: `.\build.ps1 newboard`

### Modifying Components

**Logging (`common/src/logging/`):**
- `LogBox::begin("Title")` - Start log section with title
- `LogBox::line("message")` - Log a line
- `LogBox::linef("format %d", value)` - Log with formatting
- `LogBox::end()` - End section, show elapsed time
- `LogBox::message("Title", "message")` - Single-line log

**Power Management (`common/src/power/`):**
- `begin()` - Initialize power manager
- `getWakeupReason()` - Returns WAKEUP_FIRST_BOOT, WAKEUP_TIMER, WAKEUP_BUTTON, WAKEUP_RESET_BUTTON
- `readBatteryVoltage()` - Read battery voltage (if HAS_BATTERY)
- `configureWakeButton(pin, level)` - Configure wake button
- `enableWatchdog(seconds)` - Enable watchdog timer
- `disableWatchdog()` - Disable watchdog timer
- `sleepForSeconds(seconds)` - Enter deep sleep

**Config Management (`common/src/config/`):**
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

**WiFi Management (`common/src/wifi/`):**
- `startAccessPoint()` - Start AP mode
- `getAPName()`, `getAPIPAddress()` - Get AP info
- `connectToWiFi()` - Connect using stored credentials
- `connectToWiFi(ssid, pass, &retryCount)` - Connect with specific credentials
- `getRSSI()` - Get WiFi signal strength
- `getLocalIP()` - Get local IP address
- `isConnected()` - Check connection status
- `setPowerManager(powerMgr)` - Link with power manager for channel locking

**Config Portal (`common/src/portal/`):**
- `begin(port)` - Start web server on port (default 80)
- `handleClient()` - Process HTTP requests (call in loop)
- `isConfigReceived()` - Check if config submitted
- `stop()` - Stop web server

**MQTT Manager (`common/src/mqtt/`):**
- `begin()` - Initialize MQTT manager
- `connect()` - Connect to broker
- `publishAllTelemetry(telemetryData)` - Publish all telemetry (batch)
- `publishDiscovery(telemetryData)` - Publish Home Assistant discovery
- Individual publish methods available (see mqtt_manager.h)

**TelemetryData struct fields:**
- `deviceId`, `deviceName`, `modelName` - Device identification
- `wakeReason` - Wake reason enum
- `batteryVoltage` (0.0 to skip), `batteryPercentage` (-1 to skip)
- `wifiRSSI`, `wifiBSSID` (empty to skip), `wifiRetryCount` (255 to skip)
- `loopTimeTotal`, `loopTimeWiFi`, `loopTimeWork`, `loopTimeOther`
- `otherRetryCount1`, `otherRetryCount2` (255 to skip)
- `lastLogMessage` (empty to skip), `lastLogSeverity`
- `freeHeap` (0 to skip)

## Validation & Testing

### Pre-Commit Checklist

**CRITICAL:** Do NOT claim work is complete until ALL steps are verified:

1. ✅ **Used simple includes** (no `../` relative paths in common/src/ files)
2. ✅ **Board config constants defined** (if adding new board)
3. ✅ **MANDATORY: Built all boards successfully**
   - Run: `.\build.ps1 all` (Windows) or `./build.sh all` (Linux/macOS)
   - **ALL boards MUST compile without errors**
   - If Arduino CLI not installed: Use setup instructions OR clearly state build not tested
   - **DO NOT** claim "implementation complete" without successful builds
4. ✅ **No compilation errors** in build output
5. ✅ **Firmware sizes reasonable** (<1.3MB per board - check build output)
6. ✅ **Updated documentation** if needed (README.md, CUSTOMIZATION.md)

**If you cannot build locally:**
- Clearly state: "⚠️ Local build not tested (Arduino CLI not available)"
- CI/CD will validate builds, but expect potential compilation failures
- Do NOT claim work is "complete" or "ready" without successful builds

### Manual Testing (Optional)

**Local build test:**
```bash
.\build.ps1 esp32_dev
ls build/esp32_dev/*.bin
```

## Common Issues & Solutions

### Build Failures

**Issue: "Include file not found" error**
- Cause: Using relative paths in includes
- Solution: Change `#include "../logging/logger.h"` to `#include "logger.h"`

**Issue: "PubSubClient.h not found"**
- Cause: Library not installed
- Solution: Build script should auto-install. Manually install: `arduino-cli lib install PubSubClient`

**Issue: ".cpp files not compiling"**
- Cause: Build script not copying files correctly
- Solution: Verify build script copies both `.cpp` and `.h` files recursively

### Include Path Issues

**Issue: Includes fail after adding new component**
- Cause: Using relative paths
- Solution: All includes in `common/src/` MUST use simple names:
  ```cpp
  // In common/src/mqtt/mqtt_manager.cpp:
  #include "mqtt_manager.h"     // GOOD
  #include "config_manager.h"   // GOOD
  #include "logger.h"           // GOOD
  
  // NOT:
  #include "../config/config_manager.h"  // BAD
  ```

### Board-Specific Issues

**Issue: Battery voltage always 0.0**
- Cause: `HAS_BATTERY` not defined or `BATTERY_ADC_PIN` incorrect
- Solution: Check `board_config.h` for correct battery configuration

**Issue: Deep sleep not working**
- Cause: Watchdog timer preventing sleep
- Solution: Call `powerMgr.disableWatchdog()` before sleep

## Key Commands Reference

```bash
# Build (Windows)
.\build.ps1 esp32_dev                  # Build one board
.\build.ps1 all                        # Build all boards

# Build (Linux/macOS)
./build.sh esp32_dev                   # Build one board
./build.sh all                         # Build all boards

# Upload (Windows)
.\upload.ps1 -Board esp32_dev -Port COM7

# Upload (Linux/macOS)
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 boards/esp32_dev

# Manual library install (if needed)
arduino-cli lib install PubSubClient
```

## Architecture Patterns

### Pattern: Feature-Based Organization

Each feature is self-contained in its own directory:
```
common/src/
├── logging/      # Logging system
├── power/        # Power management
├── config/       # Configuration storage
├── wifi/         # WiFi management
├── portal/       # Web configuration
├── mqtt/         # MQTT telemetry
└── modes/        # Mode controllers
```

### Pattern: Board Abstraction

Board-specific behavior is controlled via `board_config.h` constants, not hardcoded board checks:

```cpp
// GOOD - flexible and board-agnostic
#if HAS_BATTERY
  float voltage = powerMgr.readBatteryVoltage();
#endif

// BAD - hardcoded board check
#ifdef ARDUINO_ESP32S3
  float voltage = analogRead(35);
#endif
```

### Pattern: Shared Main Sketch

The template uses a shared `main_sketch.ino.inc` file that contains setup() and loop():

**Board sketch** (`boards/esp32_dev/esp32_dev.ino`) - Minimal ~20 lines:
```cpp
/*
   ESP32 Multi-Board Template - ESP32 DevKit V1
*/

// Board validation
#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_LOLIN_D32)
#error "Wrong board selection for this sketch"
#endif

// Include board configuration first (required by shared code)
#include "board_config.h"

// Include shared implementation (contains setup() and loop())
#include <src/main_sketch.ino.inc>
```

**Shared sketch** (`common/src/main_sketch.ino.inc`) - Contains actual logic:
- Global component instances
- setup() function with initialization
- loop() function with application logic
- Uses constants from board_config.h via `#if HAS_BATTERY`, `BOARD_NAME`, etc.

**CRITICAL:** When adding features, modify `main_sketch.ino.inc`, NOT individual board .ino files.

## OTA Updates (`common/src/ota/`)

Over-the-air firmware updates via config portal:

**File Upload Method:**
```cpp
#include "ota/ota_manager.h"

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

**Config Portal Integration:**
- Route: `http://<device-ip>/ota` - Shows OTA UI
- File upload: `POST /ota/upload` - Uploads firmware binary
- URL update: `POST /ota/url` - Updates from HTTP URL
- Progress tracking with watchdog management
- Automatic reboot after successful update

**Partition Requirements:**
- Must use `min_spiffs` partition scheme (1.9MB APP with OTA)
- Firmware must be < 1.5MB to fit in partition
- ESP32 dual-partition scheme provides rollback protection

## Web Flasher (GitHub Pages)

Browser-based firmware flashing using ESP Web Tools (no drivers needed!):

**Deployment:**
- Hosted on GitHub Pages at `https://username.github.io/repo/`
- Automatically deployed by release.yml workflow
- Stored in `gh-pages` branch (separate from main)

**Files:**
- `flasher/index.html` - ESP Web Tools UI
- `flasher/app.js` - Device selection logic
- `flasher/manifest_{board}.json` - Flash instructions per board
- `flasher/firmware/v{version}/` - Binary files for each release

**Usage:**
1. User visits GitHub Pages URL
2. Selects board (ESP32 DevKit V1 or ESP32-S3 DevKit)
3. Clicks "Connect" → Selects serial port (Web Serial API)
4. Clicks "Install" → Firmware downloads from GitHub Pages
5. Device flashes and reboots

**Manifest Generation:**
- `scripts/generate_manifests.sh` creates ESP Web Tools manifests
- Points to GitHub Pages URLs for binaries
- Includes bootloader offsets (ESP32: 0x1000, ESP32-S3: 0x0000)
- Auto-generated on every release

## GitHub Actions CI/CD

### Pull Request Workflow (build.yml)

**Triggers:** Pull request to `main` branch

**Jobs:**
1. **Version Check** (optional if version.h exists):
   - Compares version between PR and main
   - Validates CHANGELOG.md updated
   - Posts PR comment with guidance
   
2. **Build All Boards**:
   - Compiles esp32_dev and esp32s3_dev in parallel
   - Validates firmware size < 1.5MB
   - Uploads build artifacts
   
3. **Comment Summary**:
   - Posts table with firmware sizes
   - Shows percentage of partition used
   - Provides download links

**Version Tracking** (optional):
- Create `common/src/version.h` with `#define FIRMWARE_VERSION "1.0.0"`
- Update `CHANGELOG.md` following Keep a Changelog format
- Workflow validates version incremented on PRs with code changes

### Release Workflow (release.yml)

**Trigger:** Git tag push (format: `v*.*.*`)

**Jobs:**
1. **Validate Tag** - Ensures tag matches version.h
2. **Build All Boards** - Compiles with bootloaders and partitions
3. **Create GitHub Release** - With CHANGELOG excerpt and 12 binaries
4. **Generate Manifests** - Creates ESP Web Tools manifests
5. **Deploy to gh-pages** - Commits firmware and manifests

**Outputs:**
- GitHub Release with binaries
- Updated `gh-pages` branch with flasher
- Web flasher available at GitHub Pages URL

**Release Process:**
```bash
# After merging PR
git tag v1.2.3
git push origin v1.2.3
# Workflow automatically builds and deploys
```

### Pages Deployment (deploy-pages.yml)

**Trigger:** Push to `gh-pages` branch (automatic after release)

**Job:**
- Deploys flasher site to GitHub Pages
- Serves with proper CORS headers for Web Serial API
- Site live at `https://username.github.io/repo/`

### Branch Strategy

**main branch** (clean code, NO binaries):
- Source code (.cpp, .h, .ino)
- Build scripts
- Documentation
- Workflows
- Flasher templates (HTML, JS, CSS)

**gh-pages branch** (flasher + firmware):
- flasher/ directory with all assets
- flasher/firmware/v{version}/ with binaries
- flasher/manifest_*.json (generated)
- flasher/latest.json (generated)

## Validation & Testing

### Pre-Commit Checklist

**If version tracking enabled:**
1. ✅ Updated `common/src/version.h`
2. ✅ Updated `CHANGELOG.md` with version entry
3. ✅ Built all boards successfully: `.\build.ps1 all`
4. ✅ Firmware sizes < 1.5MB
5. ✅ Tested on hardware (optional but recommended)

**If version tracking not used:**
1. ✅ Built all boards successfully
2. ✅ Firmware sizes reasonable
3. ✅ Code compiles without errors

### CI/CD Validation

**PR checks (automatic):**
- Version increment validated (if version.h exists)
- CHANGELOG entry validated (if version changed)
- All boards build successfully
- Firmware sizes within limits
- PR comments posted with results

**Release checks (automatic on tag push):**
- Tag matches version.h
- All builds succeed
- GitHub Release created
- Manifests generated
- Flasher deployed to GitHub Pages

## Adding New Features

### Example: Adding a New Sensor

1. **Create feature directory:**
   ```
   common/src/sensors/
   ├── sensor_manager.h
   └── sensor_manager.cpp
   ```

2. **Update main_sketch.ino.inc:**
   ```cpp
   #include <src/sensors/sensor_manager.h>
   
   SensorManager sensorMgr;
   
   void setup() {
     // ... existing setup
     sensorMgr.begin();
   }
   
   void loop() {
     float value = sensorMgr.read();
   }
   ```

3. **Build and test:**
   ```powershell
   .\build.ps1 all
   ```

4. **Update documentation** if user-facing feature

## Common Issues & Solutions

### Build Failures

**Issue: "Include file not found"**
- Cause: Using relative paths (`#include "../logger.h"`)
- Solution: Use simple includes (`#include "logger.h"`)

**Issue: "Wrong board selection" error**
- Cause: Board-specific validation in .ino file
- Solution: Use build scripts with correct board name

**Issue: "Firmware too large"**
- Cause: Exceeded 1.5MB partition limit
- Solution: Optimize code, remove unused libraries, check `min_spiffs` partition

### OTA Issues

**Issue: OTA update fails**
- Cause: Firmware too large or connection timeout
- Solution: Ensure firmware < 1.5MB, stable WiFi connection

**Issue: Device doesn't reboot after OTA**
- Cause: Update failed, check serial monitor
- Solution: Verify firmware URL accessible, check logs

### Web Flasher Issues

**Issue: Can't connect to device**
- Cause: Wrong browser or missing USB drivers
- Solution: Use Chrome/Edge/Opera, install CP210x or CH340 drivers

**Issue: Flasher shows old firmware**
- Cause: GitHub Pages not updated
- Solution: Check Actions logs, verify gh-pages branch updated

### CI/CD Issues

**Issue: PR workflow doesn't run**
- Cause: Only docs changed (excluded by paths-ignore)
- Solution: This is expected behavior

**Issue: Release fails with "tag mismatch"**
- Cause: Git tag doesn't match version.h
- Solution: Ensure tag is `v1.2.3` and version.h is `"1.2.3"`

**Issue: gh-pages deployment fails**
- Cause: Missing workflow permissions
- Solution: Enable "Read and write permissions" in Settings → Actions

## Documentation

- **[README.md](../README.md)** - Main project overview
- **[FIRST_DEPLOYMENT.md](../FIRST_DEPLOYMENT.md)** - First-time deployment checklist
- **[docs/CICD_FLOW.md](../docs/CICD_FLOW.md)** - CI/CD pipeline visualization
- **[docs/PR_WORKFLOW.md](../docs/PR_WORKFLOW.md)** - PR validation guide
- **[docs/WEB_FLASHER_SETUP.md](../docs/WEB_FLASHER_SETUP.md)** - Flasher setup guide
- **[docs/CUSTOMIZATION.md](../docs/CUSTOMIZATION.md)** - Adding boards and features
- **[docs/GH_PAGES_BRANCH.md](../docs/GH_PAGES_BRANCH.md)** - Branch strategy

## Final Notes**Trust these instructions.** The project uses a non-standard Arduino pattern for code sharing. Only search for additional information if:
- Instructions are incomplete for your specific task
- You encounter an error not covered here
- User requests functionality not documented

**When in doubt:**
1. Check `docs/CUSTOMIZATION.md` for feature customization details
2. Check `boards/esp32_dev/` for working example
3. Check `README.md` for user-facing documentation
4. Review component headers in `common/src/` for API documentation

**Remember:**
- Use simple includes (no `../` paths)
- Test all boards before claiming complete
- Board configs control hardware differences
- Build script handles file copying automatically






