# Agent Prompt: Add ESP32 Board Support

## Mission
Add support for a new ESP32 board variant to the esp32-multiboard-template project. This involves researching board specifications, creating configuration files, and validating the build.

## Required Information

You must obtain these three pieces of information:

1. **Board Name** - The marketing/commercial name (e.g., "ESP32-C6 DevKit")
2. **FQBN** - Fully Qualified Board Name for Arduino CLI (e.g., `esp32:esp32:esp32c6`)
3. **Board Manager URL** - URL to the Arduino board package index JSON

## Step-by-Step Process

### Phase 1: Research Board Specifications

**Goal:** Find the FQBN and Board Manager URL for the target board.

**Search Strategy:**

1. **Check Arduino CLI** (if user has it installed):
   ```bash
   arduino-cli board listall <board-name>
   ```

2. **Search Official ESP32 Documentation:**
   - https://docs.espressif.com/projects/arduino-esp32/
   - Look for "Board Support" or "Supported Boards" sections
   - Official ESP32 boards use: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

3. **Search Vendor Documentation:**
   - If it's a third-party board (e.g., Inkplate, TTGO, Heltec), search for the manufacturer's Arduino setup guide
   - Look for phrases like "Board Manager URL" or "Additional Boards Manager URLs"
   - Common vendors have their own board manager URLs (e.g., Inkplate boards)

4. **Search GitHub:**
   - Look for the board's GitHub repository
   - Check README.md or docs/ folder for Arduino setup instructions
   - Search for "boards.txt" file which contains FQBN definitions

5. **Search Arduino Forums/Communities:**
   - https://forum.arduino.cc/
   - ESP32.com forums
   - Reddit r/esp32

**Common FQBN Patterns:**

| Board Family | Example FQBN |
|--------------|--------------|
| ESP32 Classic | `esp32:esp32:esp32` |
| ESP32-S2 | `esp32:esp32:esp32s2` |
| ESP32-S3 | `esp32:esp32:esp32s3` |
| ESP32-C3 | `esp32:esp32:esp32c3` |
| ESP32-C6 | `esp32:esp32:esp32c6` |
| Third-party boards | `<vendor>:esp32:<board>` |

**When to Ask for Help:**

If after searching the above sources you cannot find EITHER the FQBN OR the Board Manager URL, **STOP** and ask the user:

```
I need help identifying the board specifications for [BOARD_NAME]:

❓ Unable to find: [FQBN / Board Manager URL / Both]

I searched:
- [List sources you checked]

Could you provide:
1. Link to official Arduino setup documentation
2. Link to vendor's GitHub repository
3. Or directly provide the FQBN and Board Manager URL

Example format:
FQBN: esp32:esp32:esp32c6
Board Manager URL: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

### Phase 2: Create Board Configuration Files

**Goal:** Create the three required files in a new board directory.

#### File 1: Create Board Directory and board.json

Create `boards/<board_id>/board.json`:

```json
{
  "name": "<Full Board Name>",
  "fqbn": "<package>:<platform>:<board>",
  "board_manager_url": "<board_manager_url>"
}
```

**Naming Convention for `<board_id>`:**
- Lowercase, underscores for spaces
- Examples: `esp32_dev`, `esp32s3_dev`, `esp32c6_dev`, `inkplate10`

**Example:**
```json
{
  "name": "ESP32-C6 DevKit",
  "fqbn": "esp32:esp32:esp32c6",
  "board_manager_url": "https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json"
}
```

#### File 2: Create board_config.h

Create `boards/<board_id>/board_config.h`:

**Template:**
```cpp
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// Board identification
#define BOARD_NAME "<Full Board Name>"
#define BOARD_MODEL "<ESP32-XX>"  // e.g., "ESP32", "ESP32-S3", "ESP32-C6"

// Hardware configuration
#define HAS_BATTERY false       // Does board have battery monitoring?
#define BATTERY_ADC_PIN 0       // ADC pin for battery voltage (if HAS_BATTERY)
#define BATTERY_DIVIDER_R1 100  // Voltage divider R1 (kΩ)
#define BATTERY_DIVIDER_R2 100  // Voltage divider R2 (kΩ)

#define HAS_BUTTON true         // Does board have a button?
#define WAKE_BUTTON_PIN 0       // GPIO pin for wake/boot button

#define LED_PIN 2               // Built-in LED pin (commonly GPIO 2 or 8)

#endif // BOARD_CONFIG_H
```

**How to determine pin values:**

1. **LED_PIN:**
   - Check board schematic or documentation
   - Common values: GPIO 2 (classic ESP32), GPIO 8 (ESP32-C6)
   - Try searching: "[board name] led gpio" or "[board name] schematic"

2. **WAKE_BUTTON_PIN:**
   - Usually GPIO 0 (BOOT button on most ESP32 boards)
   - Check board documentation for "BOOT button" or "EN button"

3. **HAS_BATTERY:**
   - Set to `false` unless the board has built-in battery monitoring
   - Only set to `true` if board has voltage divider circuit to ADC pin

4. **BOARD_MODEL:**
   - Match the chip variant: "ESP32", "ESP32-S2", "ESP32-S3", "ESP32-C3", "ESP32-C6", etc.

**If uncertain about pin values:**
- Use defaults: `WAKE_BUTTON_PIN 0`, `LED_PIN 2`, `HAS_BATTERY false`
- These are safe defaults that work for most ESP32 DevKit boards

#### File 3: Create board sketch (.ino file)

Create `boards/<board_id>/<board_id>.ino`:

**Template:**
```cpp
/*
   ESP32 Multi-Board Template - <Full Board Name>
   
   Board: <Full Board Name>
   Chip: <ESP32-XX>
   
   This is the minimal board-specific entry point.
   All shared logic is in common/src/main_sketch.ino.inc
*/

// Include board configuration first (required by shared code)
#include "board_config.h"

// Validate board configuration
#ifndef BOARD_NAME
#error "BOARD_NAME not defined in board_config.h"
#endif

#ifndef BOARD_MODEL
#error "BOARD_MODEL not defined in board_config.h"
#endif

// Include shared implementation (contains setup() and loop())
#include "main_sketch.ino.inc"
```

**⚠️ CRITICAL:** 
- Replace `<board_id>` with the actual directory name
- Replace `<Full Board Name>` with the actual board name
- Replace `<ESP32-XX>` with the chip variant

### Phase 3: Validate Build

**Goal:** Ensure the board compiles successfully and firmware size is acceptable.

#### Build Test

Run the build script for your new board:

**Windows (PowerShell):**
```powershell
.\build.ps1 <board_id>
```

**Linux/macOS:**
```bash
./build.sh <board_id>
```

The build script will automatically:
1. Discover your new board from `board.json`
2. Install the board manager URL from `board_manager_url` field
3. Install required Arduino cores
4. Compile the firmware

#### Success Criteria

✅ **Build must succeed with:**
- No compilation errors
- Firmware size < 1.5 MB (to fit in OTA partition)
- All three files created correctly

#### Check Build Output

After successful build, verify:

```powershell
# Check build directory exists
Test-Path "build\<board_id>"

# Check binary files exist
Test-Path "build\<board_id>\<board_id>.ino.bin"

# Check firmware size (should be < 1.5MB)
(Get-Item "build\<board_id>\<board_id>.ino.bin").Length / 1MB
```

#### Common Build Issues and Solutions

**Issue: "Board not found" or "Unknown FQBN"**
- **Cause:** FQBN is incorrect or board package not installed
- **Solution:** Double-check FQBN format, verify board manager URL is correct

**Issue: "Include file not found"**
- **Cause:** Using relative paths or missing board_config.h validation
- **Solution:** Ensure board_config.h exists and has all required #defines

**Issue: "Package index download failed"**
- **Cause:** Board manager URL is incorrect or inaccessible
- **Solution:** Verify URL is correct, try accessing it in browser

**Issue: Firmware size > 1.5MB**
- **Cause:** Debug symbols or large libraries
- **Solution:** This is expected for first build, can be optimized later

**If build fails with unclear errors:**
Ask the user for help with the specific error message:

```
Build failed for [BOARD_NAME]:

Error: [paste error message]

Build command used: [command]

Could you help diagnose this issue?
```

### Phase 4: Documentation (Optional)

After successful build, you may optionally suggest:

1. **Update README.md** to mention the new board
2. **Add example to docs/CUSTOMIZATION.md** if it's a unique/interesting board
3. **Create PR** if working in a fork

## Complete Checklist

Before marking the task complete, verify:

- [ ] `boards/<board_id>/board.json` created with valid FQBN and board_manager_url
- [ ] `boards/<board_id>/board_config.h` created with all required defines
- [ ] `boards/<board_id>/<board_id>.ino` created with correct includes
- [ ] Build script runs successfully: `.\build.ps1 <board_id>`
- [ ] Firmware binary created in `build/<board_id>/<board_id>.ino.bin`
- [ ] Firmware size is reasonable (< 1.5MB preferred)
- [ ] No compilation errors or warnings

## Example: Adding ESP32-C6 DevKit

**Research Results:**
- Board Name: "ESP32-C6 DevKit"
- FQBN: `esp32:esp32:esp32c6`
- Board Manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

**Created Files:**

1. `boards/esp32c6_dev/board.json`:
```json
{
  "name": "ESP32-C6 DevKit",
  "fqbn": "esp32:esp32:esp32c6",
  "board_manager_url": "https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json"
}
```

2. `boards/esp32c6_dev/board_config.h`:
```cpp
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define BOARD_NAME "ESP32-C6 DevKit"
#define BOARD_MODEL "ESP32-C6"

#define HAS_BATTERY false
#define BATTERY_ADC_PIN 0
#define BATTERY_DIVIDER_R1 100
#define BATTERY_DIVIDER_R2 100

#define HAS_BUTTON true
#define WAKE_BUTTON_PIN 9  // ESP32-C6 boot button

#define LED_PIN 8  // ESP32-C6 typical LED pin

#endif
```

3. `boards/esp32c6_dev/esp32c6_dev.ino`:
```cpp
/*
   ESP32 Multi-Board Template - ESP32-C6 DevKit
*/

#include "board_config.h"

#ifndef BOARD_NAME
#error "BOARD_NAME not defined in board_config.h"
#endif

#include "main_sketch.ino.inc"
```

**Build Test:**
```powershell
.\build.ps1 esp32c6_dev
# ✅ Build succeeded, firmware size: 1.2MB
```

## Important Notes

1. **Build scripts auto-discover boards** - Local builds don't need manual script editing
2. **CI/CD requires manual updates** - GitHub Actions workflows need board added to matrix (not your responsibility as agent, document this for user)
3. **All includes use simple names** - Never use relative paths like `../` in includes
4. **Board IDs are unique** - Choose names that don't conflict with existing boards

## References

- Project Documentation: `docs/DEVELOPER_GUIDE.md`
- Customization Guide: `docs/CUSTOMIZATION.md`
- Example Boards: `boards/esp32_dev/`, `boards/esp32s3_dev/`
- Build Scripts: `build.ps1` (Windows), `build.sh` (Linux/macOS)
