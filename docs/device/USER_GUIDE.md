# Device User Guide

Complete guide for using ESP32 devices running firmware based on this template.

## Table of Contents

- [First Boot Setup](#first-boot-setup)
- [WiFi Configuration](#wifi-configuration)
- [Configuration Portal](#configuration-portal)
- [MQTT Integration](#mqtt-integration)
- [Firmware Updates](#firmware-updates)
- [Troubleshooting](#troubleshooting)

---

## First Boot Setup

When you power on your device for the first time (or after a factory reset), it will automatically enter configuration mode.

### What Happens on First Boot

1. **Device creates WiFi access point**
   - Network name: `esp32-XXXXXX` (where XXXXXX is part of the device's MAC address)
   - No password required
   - IP address: `192.168.4.1`

2. **LED indicator** (if available)
   - Typically blinks to show it's in AP mode
   - Check your device documentation for specific LED patterns

3. **Device waits for configuration**
   - Will remain in AP mode until configured
   - Does not connect to any WiFi network yet

---

## WiFi Configuration

### Step 1: Connect to Device Access Point

**On Windows:**
1. Click WiFi icon in taskbar
2. Select network `esp32-XXXXXX`
3. Click "Connect"
4. Wait for connection (no password needed)

**On macOS:**
1. Click WiFi icon in menu bar
2. Select network `esp32-XXXXXX`
3. Wait for connection

**On iPhone/Android:**
1. Open Settings → WiFi
2. Select network `esp32-XXXXXX`
3. Connect (no password needed)

### Step 2: Open Configuration Portal

1. Open web browser (Chrome, Firefox, Safari, Edge)
2. Navigate to: `http://192.168.4.1`
3. Configuration page should load automatically

**If page doesn't load:**
- Try `http://192.168.4.1/`
- Check you're still connected to device AP
- Try different browser
- Disable mobile data (on phones) to force traffic through WiFi

### Step 3: Enter WiFi Credentials

1. **WiFi Network Name (SSID)**
   - Enter your home/office WiFi network name
   - Must match exactly (case-sensitive)

2. **WiFi Password**
   - Enter your WiFi password
   - Must match exactly (case-sensitive)
   - Click "show password" icon to verify

3. **Device Name** (optional but recommended)
   - Give your device a friendly name
   - Example: "bedroom-sensor", "garage-monitor", "living-room"
   - Used in MQTT topics and Home Assistant

### Step 4: Advanced Settings (Optional)

#### Static IP Configuration

If you want your device to always use the same IP address:

1. Check "Use Static IP"
2. Enter:
   - **IP Address**: Device IP (e.g., `192.168.1.100`)
   - **Gateway**: Router IP (e.g., `192.168.1.1`)
   - **Subnet Mask**: Usually `255.255.255.0`
   - **DNS Server 1**: Usually `8.8.8.8` (Google DNS)
   - **DNS Server 2**: Usually `8.8.4.4` (Google DNS)

**When to use static IP:**
- You need to access device at predictable address
- Your router doesn't support DHCP reservations
- You're integrating with systems that require fixed IPs

**When to use DHCP (automatic):**
- Most home networks (recommended)
- You access device through Home Assistant (uses MQTT)
- Your router supports DHCP reservations

#### MQTT Configuration

If you're using Home Assistant or MQTT-based automation:

1. **MQTT Broker URL**
   - Format: `mqtt://192.168.1.10:1883`
   - Replace IP with your MQTT broker address
   - Port is usually `1883` (unencrypted) or `8883` (encrypted)

2. **MQTT Username** (optional)
   - If your broker requires authentication
   - Leave blank if no authentication

3. **MQTT Password** (optional)
   - If your broker requires authentication
   - Leave blank if no authentication

### Step 5: Save Configuration

1. Click **"Save Configuration"** button
2. Device will:
   - Save settings to flash memory
   - Close access point
   - Attempt to connect to your WiFi
   - Reboot

3. **Wait 30-60 seconds** for device to:
   - Connect to WiFi
   - Get IP address
   - Initialize MQTT (if configured)
   - Start normal operation

---

## Configuration Portal

Once your device is connected to WiFi, you can access the configuration portal anytime.

### Finding Device IP Address

**Method 1: Check Router**
- Log into your router admin page
- Look for connected devices list
- Find device by name (if you set friendly name)
- Note the IP address

**Method 2: MQTT/Home Assistant**
- If using MQTT, check Home Assistant for device
- Device IP shown in device info

**Method 3: Network Scanner App**
- Use app like "Fing" (iOS/Android) or "Advanced IP Scanner" (Windows)
- Scan your network
- Look for device with name matching your configuration

### Accessing Portal

1. Open web browser
2. Navigate to: `http://<device-ip>/`
3. Example: `http://192.168.1.100/`

### Portal Features

#### Home Page (`/`)
- View current configuration
- Modify WiFi settings
- Change device name
- Update MQTT settings
- Change static IP configuration

#### OTA Update Page (`/ota`)
- Upload firmware file (.bin)
- Update from URL
- Check current firmware version
- See update progress

### Re-entering Configuration Mode

If you need to reconfigure the device:

**Method 1: Button Press (if device has button)**
- Hold wake/config button during power-on
- Release when LED starts blinking
- Device enters AP mode

**Method 2: WiFi Failure**
- Device automatically enters AP mode if:
  - Can't connect to saved WiFi network
  - WiFi credentials are incorrect
  - Network is unavailable

**Method 3: Factory Reset (if supported)**
- Check device documentation
- Usually involves holding button for 10+ seconds
- Erases all saved configuration

---

## MQTT Integration

If you configured MQTT, your device publishes telemetry data automatically.

### Home Assistant Auto-Discovery

**Device automatically appears in Home Assistant when:**
1. MQTT broker is configured correctly
2. Home Assistant is connected to same MQTT broker
3. Home Assistant MQTT integration is enabled
4. Device successfully connects to broker

**What you'll see in Home Assistant:**
- Device card with friendly name
- Sensors for:
  - Battery voltage (if battery-powered)
  - Battery percentage
  - WiFi signal strength (RSSI)
  - WiFi access point (BSSID)
  - Last wake reason
  - Loop timing metrics
  - Free heap memory
  - Last log message
  - Device state

### MQTT Topics

Your device publishes to these topics:

**State topic:**
```
homeassistant/sensor/<device-id>/state
```

**Discovery topics:**
```
homeassistant/sensor/<device-id>_<metric>/config
```

**Example topics:**
```
homeassistant/sensor/esp32-a1b2c3/state
homeassistant/sensor/esp32-a1b2c3_battery_voltage/config
homeassistant/sensor/esp32-a1b2c3_rssi/config
```

### Published Metrics

| Metric | Description | Unit |
|--------|-------------|------|
| Battery Voltage | Current battery voltage | Volts |
| Battery Percentage | Estimated battery % | % |
| WiFi RSSI | Signal strength | dBm |
| WiFi BSSID | Access point MAC | - |
| Wake Reason | Why device woke up | - |
| Loop Time Total | Total processing time | seconds |
| Loop Time WiFi | WiFi connection time | seconds |
| Free Heap | Available memory | bytes |
| Last Log | Last log message | - |

---

## Firmware Updates

### Over-the-Air (OTA) Updates

You can update device firmware remotely without USB cable.

#### Method 1: File Upload

**Prerequisites:**
- New firmware file (`.bin` file)
- Device connected to WiFi
- Access to configuration portal

**Steps:**
1. Get firmware file from:
   - Project releases page on GitHub
   - Web flasher download
   - Build it yourself

2. Access OTA page:
   - Open browser to `http://<device-ip>/ota`

3. Upload firmware:
   - Click "Choose File"
   - Select `.bin` file
   - Click "Update"
   - Wait for upload (30-60 seconds)

4. Device will:
   - Verify firmware
   - Flash to alternate partition
   - Reboot automatically
   - Run new firmware

**Progress indicator:**
- Shows upload percentage
- Shows flash progress
- Displays success/error messages

#### Method 2: Update from URL

**Prerequisites:**
- Firmware hosted on HTTP server
- URL to firmware file
- Device connected to WiFi

**Steps:**
1. Access OTA page: `http://<device-ip>/ota`

2. Enter firmware URL:
   - Example: `https://yourusername.github.io/repo/firmware/v1.0.1/esp32_dev.ino.bin`
   - URL must be HTTP or HTTPS
   - File must be accessible without authentication

3. Click "Update from URL"

4. Device will:
   - Download firmware
   - Verify checksum
   - Flash to partition
   - Reboot

**Advantages:**
- No file download needed
- Can update multiple devices easily
- Automated update scripts possible

### Web Flasher (USB Required)

For major updates or recovery:

1. Visit project's GitHub Pages:
   - `https://yourusername.github.io/project-name/`

2. Connect device via USB

3. Select your board type

4. Click "Connect" and select serial port

5. Click "Install" to flash firmware

**When to use web flasher:**
- Device won't boot
- OTA update failed
- Need to flash from scratch
- Changing board types

---

## Troubleshooting

### WiFi Issues

**Device won't connect to WiFi:**
- Verify WiFi credentials (case-sensitive)
- Check WiFi network is 2.4GHz (not 5GHz)
- Ensure router is broadcasting SSID
- Check WiFi password is correct
- Move device closer to router
- Check router MAC filtering settings

**Device disconnects frequently:**
- Weak signal (check RSSI in Home Assistant)
- Router issues (reboot router)
- Interference from other devices
- Power supply issues

**Can't find device AP on first boot:**
- Wait 30 seconds after power-on
- Check device LED (should indicate AP mode)
- Try power cycling device
- Check device is not already configured

### Configuration Portal Issues

**Can't access portal at 192.168.4.1:**
- Verify connected to device AP
- Disable mobile data (on phones)
- Try different browser
- Clear browser cache
- Try `http://192.168.4.1/` with trailing slash

**Portal loads but won't save config:**
- Check all required fields filled
- Verify WiFi password correct
- Try shorter device name
- Check browser console for errors

**Device reboots but doesn't connect:**
- WiFi credentials incorrect
- Network unavailable
- Router blocking device
- Static IP conflict (if using static IP)

### MQTT Issues

**Device not appearing in Home Assistant:**
- Check MQTT broker running
- Verify broker IP/port correct
- Check MQTT credentials (if used)
- Restart Home Assistant MQTT integration
- Check Home Assistant MQTT configuration

**Sensors show "unavailable":**
- Device not publishing (check WiFi)
- MQTT broker down
- Topic configuration changed
- Device sleeping (battery-powered)

**Old data in Home Assistant:**
- Device in deep sleep (battery mode)
- Update interval configured (check firmware)
- MQTT broker issues
- Network latency

### OTA Update Issues

**Upload fails:**
- File too large (must be < 1.5MB)
- WiFi disconnected during upload
- Insufficient flash memory
- Wrong firmware file (different board)

**Device won't boot after update:**
- Firmware incompatible
- Flash corruption
- Use web flasher to recover
- Re-flash via USB

**Update progress stuck:**
- Refresh page
- Check device logs (if accessible)
- Wait 5 minutes then power cycle
- Use web flasher if unrecoverable

### Power & Battery Issues

**Battery drains quickly:**
- WiFi connection issues (constant retries)
- Deep sleep not working
- Check wake interval settings
- Battery may be old/damaged

**Device won't wake from sleep:**
- Deep sleep timeout too long
- Wake button not configured
- Hardware issue with wake pin
- Power supply issues

### General Issues

**Device keeps resetting:**
- Power supply insufficient
- Watchdog timer triggering
- Stack overflow (firmware bug)
- Hardware fault

**LED not working as expected:**
- LED pin configuration
- Check board documentation
- Firmware may use LED differently

**Can't recover device:**
1. Connect via USB
2. Use web flasher
3. Flash known-good firmware
4. Reconfigure from scratch

---

## Getting Help

**Check device logs:**
- Connect via USB
- Open serial monitor (115200 baud)
- Watch boot messages
- Look for error messages

**Documentation:**
- Check project README
- Review developer documentation
- Search GitHub issues

**Community support:**
- Open issue on GitHub
- Include:
  - Device model
  - Firmware version
  - Error messages
  - Steps to reproduce

**Reset to factory defaults:**
- Last resort option
- Erases all configuration
- Requires reconfiguration
- May fix persistent issues

---

## Best Practices

### WiFi
- Use strong WiFi signal (RSSI > -70 dBm)
- Avoid WiFi congestion
- Use DHCP unless static IP needed
- Keep router firmware updated

### MQTT
- Use descriptive device names
- Configure reliable MQTT broker
- Monitor MQTT logs
- Use WiFi password protection

### Updates
- Keep firmware up to date
- Test OTA on one device first
- Backup configuration before updates
- Don't interrupt OTA process

### Battery (if applicable)
- Monitor battery voltage
- Replace batteries proactively
- Use quality batteries
- Check for battery leakage

---

## Advanced Features

### Deep Sleep Mode

Battery-powered devices may use deep sleep to conserve power:

**How it works:**
- Device wakes up periodically
- Connects to WiFi
- Publishes telemetry
- Goes back to sleep

**Wake sources:**
- Timer (e.g., every hour)
- Button press
- External trigger

**What this means for you:**
- Sensors update on schedule, not continuously
- Device may seem "offline" between updates
- This is normal and conserves battery

### Channel Locking

For faster WiFi reconnection:

**How it works:**
- Device saves WiFi channel on first connect
- Uses same channel on subsequent connects
- Speeds up connection (3-5 seconds instead of 10-15 seconds)

**What this means for you:**
- Faster wake cycles
- Better battery life
- May fail if router changes channels (device will re-scan)

---

## Specifications

**WiFi:**
- 2.4 GHz only (not 5 GHz)
- WPA/WPA2 security supported
- Static IP or DHCP

**MQTT:**
- Protocol version 3.1.1
- QoS 0 (at most once)
- Unencrypted (port 1883)

**OTA:**
- Max firmware size: 1.5MB
- HTTP/HTTPS download supported
- Dual-partition safety (rollback on fail)

**Power:**
- Varies by device
- Check device documentation
- Battery monitoring if supported
