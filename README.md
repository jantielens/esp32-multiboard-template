# ESP32 Multi-Board Template

Production-ready ESP32 firmware template with WiFi management, MQTT telemetry, OTA updates, and power management. Build once, deploy to multiple ESP32 board variants.

## ✨ Features

- 🔌 **Multi-board architecture** - One codebase for ESP32, ESP32-S3, ESP32-C3, and more
- 📡 **WiFi Management** - AP mode setup + client mode with channel locking for fast reconnects
- 🌐 **Web Configuration Portal** - Browser-based WiFi, static IP, and MQTT configuration
- 📊 **MQTT Telemetry** - Home Assistant auto-discovery with comprehensive metrics
- 🔋 **Power Management** - Deep sleep, wake detection, battery monitoring, watchdog timer
- 🔄 **OTA Updates** - File upload and HTTP URL-based firmware updates
- 🌐 **Web Flasher** - Flash firmware from browser (no drivers needed!)
- 🛠️ **Automated CI/CD** - GitHub Actions build validation and deployment

## 🚀 Quick Start

### For Developers (Building Firmware)

```bash
# Use this template to create your own repository on GitHub:
# 1. Click "Use this template" → "Create a new repository"
# 2. Clone your new repository:
git clone https://github.com/yourusername/your-new-repo.git
cd your-new-repo

# Build firmware (Windows)
.\build.ps1 esp32_dev

# Build firmware (Linux/macOS)
./build.sh esp32_dev

# Upload (Windows)
.\upload.ps1 -Board esp32_dev -Port COM7
```

**💡 Pro Tip:** Use GitHub Copilot to explore, customize, and develop your firmware! This repository includes comprehensive instructions in `.github/copilot-instructions.md` that reference all documentation, making it easy to get AI-assisted guidance on components, APIs, and best practices.

**See [Developer Guide](docs/DEVELOPER_GUIDE.md) for complete development documentation.**

### For Users (Flashing Devices)

**Option 1: Web Flasher (Recommended)**
1. Visit [https://jantielens.github.io/esp32-multiboard-template/](https://jantielens.github.io/esp32-multiboard-template/)
2. Connect device via USB
3. Click "Install" to flash firmware

_Note: When you create your own project from this template, the web flasher will be automatically deployed to `https://YOUR-USERNAME.github.io/YOUR-REPO-NAME/` after your first release!_

**Option 2: Download Binaries**
- Visit [Releases](../../releases) page
- Download firmware for your board
- Flash using your preferred tool

**See [Device User Guide](docs/device/USER_GUIDE.md) for setup and usage instructions.**

## 📚 Documentation

### For Device Users
- **[Device User Guide](docs/device/USER_GUIDE.md)** - First boot, WiFi setup, MQTT, OTA updates

### For Template Developers
- **[Developer Guide](docs/DEVELOPER_GUIDE.md)** - Components, API reference, architecture
- **[Customization Guide](docs/CUSTOMIZATION.md)** - Adding boards and features
- **[PR Workflow](docs/PR_WORKFLOW.md)** - Pull request validation
- **[CI/CD Flow](docs/CICD_FLOW.md)** - Complete automation pipeline
- **[Web Flasher Setup](docs/WEB_FLASHER_SETUP.md)** - GitHub Pages deployment

## 🎯 Use Cases

**Perfect for:**
- IoT sensor nodes with MQTT reporting
- Battery-powered devices with deep sleep
- Multi-board product lines
- Home Assistant integrated devices
- Remote firmware updates

## License

MIT License - See LICENSE file for details
