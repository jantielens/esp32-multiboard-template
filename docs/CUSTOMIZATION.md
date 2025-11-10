# Customization Guide

**Note:** Before customizing, create your own repository using the "Use this template" button on GitHub.

## Customizing Project Name and Branding

All project metadata (name, description, repository) is defined in **`package.json`** at the project root. This is the single source of truth for the web flasher and other components.

### Edit package.json

Edit **`package.json`** in the project root:

```json
{
  "name": "esp32-multiboard-template",
  "displayName": "ESP32 Multi-Board Template",
  "displayNameShort": "ESP32 Template"
}
```

**Key fields:**
- `displayName` - Full project name (shown in header, page title of web flasher)
- `displayNameShort` - Short name (used in buttons like "Install ESP32 Template")

**Note:** The `description` and `repository` fields are automatically pulled from your GitHub repository settings during deployment, so you don't need to maintain them in package.json.

### How It Works

During deployment, the CI/CD workflow automatically generates `flasher/config.js`:

1. You edit `package.json` in main branch (for display names)
2. You edit GitHub repository description (Settings → About section)
3. On release (git tag), CI/CD workflow:
   - Reads `package.json` with `jq` for display names
   - Fetches repository description and URL from GitHub API
   - Generates `flasher/config.js` with combined metadata
   - Deploys to gh-pages branch
4. Web flasher loads `config.js` and updates all page elements dynamically

**Note:** `flasher/config.js` is auto-generated and should NOT be committed (it's in `.gitignore`).

### Local Testing

If you want to test the flasher locally before deployment:

1. Generate `config.js` manually:
   ```bash
   # Create flasher/config.js with your values
   cat > flasher/config.js << 'EOF'
   window.PROJECT_CONFIG = {
     displayName: "Your Project Name",
     displayNameShort: "Your Project",
     repository: "yourusername/your-repo",
     description: "Your project description here"
   };
   EOF
   ```

2. Serve the flasher directory:
   ```bash
   cd flasher
   python -m http.server 8000
   # Visit http://localhost:8000
   ```

3. **Don't commit** `flasher/config.js` - it's auto-generated during deployment

---

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

### 3. Create Board Metadata

Create `boards/my_new_board/board.json`:

```json
{
  "name": "My New Board",
  "fqbn": "esp32:esp32:esp32c3",
  "board_manager_url": "https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json"
}
```

**Note:** 
- The `board_manager_url` is required for CI/CD workflows to automatically install the correct Arduino core
- For official ESP32 boards, use: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- For third-party boards (e.g., Inkplate), use the vendor's board manager URL

**Important:** The `fqbn` (Fully Qualified Board Name) tells Arduino CLI which board to compile for. See the FQBN table below for common values.

### 4. Create Minimal Sketch

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

### 5. Build and Test

The build scripts automatically discover all boards by scanning the `boards/` directory and reading each `board.json` file. They also automatically discover and configure board manager URLs from the `board_manager_url` field. No manual configuration needed!

```bash
.\build.ps1 my_new_board
```

The build script will:
1. Discover your new board from `board.json`
2. Extract the `board_manager_url` and add it to Arduino CLI config
3. Install the required Arduino core (e.g., `Inkplate_Boards:esp32`)
4. Compile the firmware

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

**Issue:** Library not installed or missing from `libraries.txt`.

**Solution:** Build script should auto-install. Check `libraries.txt` has the library listed. If not, manually install:
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
- Review [Developer Guide](DEVELOPER_GUIDE.md) for component APIs
- Open an issue on GitHub
