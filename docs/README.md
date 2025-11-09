# ESP32 Multi-Board Template - Documentation

Complete documentation for the ESP32 multi-board template project.

## 📚 Documentation Index

### For Device Users
- **[Device User Guide](device/USER_GUIDE.md)** - First boot, WiFi setup, MQTT, OTA updates, troubleshooting

### For Template Developers

#### Getting Started
- **[Main README](../README.md)** - Project overview and quick start
- **[Developer Guide](DEVELOPER_GUIDE.md)** - Complete development guide with components, API, and architecture
- **[First Deployment Checklist](../FIRST_DEPLOYMENT.md)** - First-time deployment steps

#### Customization & Workflows
- **[Customization Guide](CUSTOMIZATION.md)** - Adding boards and features
- **[PR Workflow](PR_WORKFLOW.md)** - Pull request validation and versioning
- **[CI/CD Flow](CICD_FLOW.md)** - Complete automation pipeline
- **[Web Flasher Setup](WEB_FLASHER_SETUP.md)** - Browser-based firmware flashing

#### Advanced Topics
- **[GitHub Pages Branch](GH_PAGES_BRANCH.md)** - Branch strategy and gh-pages management

---

## 🔄 Quick Navigation by Task

### "I want to use a device with this firmware"
→ Start with **[Device User Guide](device/USER_GUIDE.md)**

### "I want to build firmware with this template"
→ Start with **[Developer Guide](DEVELOPER_GUIDE.md)**

### "I want to add a new ESP32 board"
→ See **[Customization Guide](CUSTOMIZATION.md)**

### "I want to set up GitHub Pages flasher"
→ Follow **[Web Flasher Setup](WEB_FLASHER_SETUP.md)**

### "I want to understand the CI/CD pipeline"
→ Read **[CI/CD Flow](CICD_FLOW.md)**

---

## 🔍 Documentation Organization

**Two distinct audiences:**

1. **Device Users** (`/docs/device/`)
   - People using devices running firmware built from this template
   - Focus: WiFi setup, MQTT configuration, OTA updates, troubleshooting
   - No programming knowledge required

2. **Template Developers** (`/docs/`)
   - People building firmware projects using this template
   - Focus: Components, APIs, build system, customization, CI/CD
   - Requires programming knowledge

---

## 📦 Quick Reference

### Build Commands

**Windows:**
```powershell
.\build.ps1 esp32_dev        # Build one board
.\build.ps1 all              # Build all boards
```

**Linux/macOS:**
```bash
./build.sh esp32_dev         # Build one board
./build.sh all               # Build all boards
```

### Project Structure

```
esp32-multiboard-template/
├── boards/                  # Board-specific sketches
├── common/src/              # Shared library code
├── flasher/                 # Web flasher
├── scripts/                 # Build automation
├── .github/workflows/       # CI/CD
└── docs/                    # Documentation
    ├── device/              # End-user docs
    └── *.md                 # Developer docs
```

---

## 🆘 Getting Help
│       └── modes/          # Mode controllers
├── flasher/                # Web flasher (deployed to GitHub Pages)
├── scripts/                # Build and deployment scripts
├── .github/workflows/      # GitHub Actions CI/CD
└── docs/                   # Documentation (this folder)
    ├── CUSTOMIZATION.md
    ├── PR_WORKFLOW.md
    ├── WEB_FLASHER_SETUP.md
```

---

## 🆘 Getting Help

1. **Check documentation** - Most questions answered here
2. **Review examples** - See `boards/` directory for working code
3. **Check workflow logs** - GitHub Actions tab shows build details
4. **Create an issue** - For bugs or feature requests

## 🔗 Quick Links

- [Arduino CLI Documentation](https://arduino.github.io/arduino-cli/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [ESP Web Tools](https://esphome.github.io/esp-web-tools/)
- [Keep a Changelog](https://keepachangelog.com/)
- [Semantic Versioning](https://semver.org/)

## 📝 License

MIT License - See [LICENSE](../LICENSE) file for details.

