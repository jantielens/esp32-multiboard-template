# Agent Prompt: Add ESP32 Board Support

## Mission
Add support for a new ESP32 board variant to the esp32-multiboard-template project. This involves researching board specifications, creating configuration files, and validating the build.

## ⚠️ Common Pitfalls to Avoid

1. **❌ DON'T guess the FQBN** - Use `arduino-cli board listall` to get the exact value
2. **❌ DON'T make 20+ web requests** - If 2-3 documentation searches fail, ask the user
3. **❌ DON'T spend hours researching pin definitions** - Use safe defaults (GPIO 0 for button, GPIO 2 for LED)
4. **❌ DON'T manually type FQBNs** - Copy-paste exactly (case-sensitive!)
5. **✅ DO use Arduino CLI first** - It's the fastest and most reliable method

## Required Information

You must obtain these three pieces of information:

1. **Board Name** - The marketing/commercial name (e.g., "ESP32-C6 DevKit")
2. **FQBN** - Fully Qualified Board Name for Arduino CLI (e.g., `esp32:esp32:esp32c6`)
3. **Board Manager URL** - URL to the Arduino board package index JSON

## Step-by-Step Process

### Phase 1: Research Board Specifications

**Goal:** Find the FQBN and Board Manager URL for the target board.

**🎯 PRIMARY METHOD: Use Arduino CLI First**

**CRITICAL:** Always try this FIRST before web searching! This is the fastest and most reliable method.

1. **Search for Board Package Online:**
   - Search: "[board name] arduino board manager url"
   - Look for the board's official setup guide
   - Find the "Additional Board Manager URLs" section
   - Copy the board manager URL (usually a `.json` file)

2. **Install the Board Package:**
   ```powershell
   # Add the board manager URL
   arduino-cli config add board_manager.additional_urls <board_manager_url>
   
   # Update the package index
   arduino-cli core update-index
   
   # Install the board package (if you know the package name)
   arduino-cli core install <package>:<platform>
   ```

3. **List Available Boards to Find FQBN:**
   ```powershell
   # Search for your board by name (case-insensitive)
   arduino-cli board listall <board-name-keyword>
   
   # Example: arduino-cli board listall inkplate
   # Example: arduino-cli board listall esp32c6
   # Example: arduino-cli board listall ttgo
   ```
   
   This command will output the exact FQBN and board name:
   ```
   Board Name                    FQBN
   Soldered Inkplate 5 V2       Inkplate_Boards:esp32:Inkplate5V2
   ```

**⚠️ Common Pitfall:** Case sensitivity matters! `Inkplate5V2` ≠ `inkplate5v2`

**Alternative Search Methods (if Arduino CLI unavailable):**

4. **Search Official ESP32 Documentation:**
   - https://docs.espressif.com/projects/arduino-esp32/
   - Look for "Board Support" or "Supported Boards" sections
   - Official ESP32 boards use: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

5. **Search Vendor Documentation:**
   - Search: "[board name] arduino setup" or "[board name] getting started arduino"
   - Look for phrases like "Board Manager URL" or "Additional Boards Manager URLs"
   - Check the vendor's GitHub repository (usually has setup instructions in README)

6. **Search GitHub for boards.txt:**
   - Look for the board's Arduino core repository
   - Navigate to `boards.txt` file (usually in `boards/` or `variants/` directory)
   - Search for your board's ID in this file
   - FQBN format: `<package>:<platform>:<board_id_from_boards.txt>`

**Example Workflow:**

```powershell
# User asks to add "inkplate 5 gen 2"

# Step 1: Search for board manager URL
# Found: https://github.com/SolderedElectronics/Dasduino-Board-Definitions-for-Arduino-IDE/raw/master/package_Dasduino_Boards_index.json

# Step 2: Add to Arduino CLI config
arduino-cli config add board_manager.additional_urls https://github.com/SolderedElectronics/Dasduino-Board-Definitions-for-Arduino-IDE/raw/master/package_Dasduino_Boards_index.json

# Step 3: Update index
arduino-cli core update-index

# Step 4: Search for board
arduino-cli board listall inkplate

# Output shows exact FQBN:
# Soldered Inkplate 5 V2    Inkplate_Boards:esp32:Inkplate5V2
#                           ^^^ Use this exact FQBN ^^^
```

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

If after using Arduino CLI and checking 2-3 vendor documentation sources you cannot find EITHER the FQBN OR the Board Manager URL, **STOP** and ask the user:

```
I need help identifying the board specifications for [BOARD_NAME]:

❓ Unable to find: [FQBN / Board Manager URL / Both]

I searched:
- [List sources you checked - max 3-4 sources]

Could you provide:
1. Link to official Arduino setup documentation
2. Or directly provide the FQBN and Board Manager URL

Example format:
FQBN: esp32:esp32:esp32c6
Board Manager URL: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

**⚠️ IMPORTANT: Avoid Excessive Web Scraping**
- Do NOT fetch 20+ web pages trying to find pin definitions
- Do NOT search through entire GitHub repositories looking for boards.txt
- If Arduino CLI doesn't work and 2-3 documentation searches fail, ASK THE USER
- Pin definitions can use safe defaults (see board_config.h section)

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

**⚠️ CRITICAL: Use Safe Defaults, Don't Over-Research**

Most ESP32 boards follow standard conventions. Use these defaults unless you find specific documentation to the contrary:

**Default Configuration (works for 95% of boards):**
```cpp
#define HAS_BATTERY false       // Only true if explicitly documented
#define BATTERY_ADC_PIN 0       // Irrelevant if HAS_BATTERY is false
#define BATTERY_DIVIDER_R1 100  // Irrelevant if HAS_BATTERY is false
#define BATTERY_DIVIDER_R2 100  // Irrelevant if HAS_BATTERY is false
#define HAS_BUTTON true         // Most boards have a BOOT button
#define WAKE_BUTTON_PIN 0       // GPIO 0 is standard BOOT button
#define LED_PIN 2               // GPIO 2 is common built-in LED
```

**Only research pins if:**
1. Board vendor explicitly documents non-standard pins
2. Board has unique features (e.g., built-in battery monitoring)
3. Quick search finds a schematic/pinout diagram

**Where to look (spend MAX 5 minutes):**
- Board's product page or datasheet
- Vendor's example code (check `setup()` function for pin definitions)
- Search: "[board name] pinout" or "[board name] schematic"

**Specific pin guidance:**

1. **LED_PIN:**
   - Try: GPIO 2 (classic ESP32), GPIO 8 (ESP32-C6), GPIO 48 (ESP32-S3)
   - If unsure: use `2` (safe default)
   - If board has no LED: set to `-1` and `HAS_LED false`

2. **WAKE_BUTTON_PIN:**
   - Almost always GPIO 0 (BOOT button)
   - Exception: Some boards use GPIO 9 or custom pins (documented by vendor)

3. **HAS_BATTERY:**
   - Set to `false` unless board explicitly has battery monitoring
   - Examples with battery: Inkplate boards, T-Display boards
   - Don't guess - battery monitoring requires hardware voltage divider

4. **BOARD_MODEL:**
   - Match the chip variant: "ESP32", "ESP32-S2", "ESP32-S3", "ESP32-C3", "ESP32-C6", etc.
   - This should match the chipFamily from your FQBN research

**If uncertain about pin values:**
- ✅ USE DEFAULTS: `WAKE_BUTTON_PIN 0`, `LED_PIN 2`, `HAS_BATTERY false`
- ✅ These work for most ESP32 DevKit boards
- ❌ DON'T spend hours searching for exact pins
- ❌ DON'T fetch 10+ web pages looking for schematics

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
- **Cause:** FQBN is incorrect (often due to case sensitivity) or board package not installed
- **Solution:** 
  1. Re-run `arduino-cli board listall <board-name>` to get EXACT FQBN (case-sensitive!)
  2. Copy the FQBN character-for-character (don't type it manually)
  3. Common mistake: `inkplate5v2` vs `Inkplate5V2` - case matters!

**Issue: "Package index download failed"**
- **Cause:** Board manager URL is incorrect or inaccessible
- **Solution:** 
  1. Verify URL is correct (try accessing it in browser - should return JSON)
  2. Check for typos in `board.json`
  3. Some URLs need `/raw/` path (GitHub repos)

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
