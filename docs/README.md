# ESP32 Multi-Board Template - Documentation

Complete documentation for the ESP32 multi-board template project.

## 📚 Documentation Index

### Getting Started
- **[Main README](../README.md)** - Project overview and quick start guide
- **[First Deployment Checklist](../FIRST_DEPLOYMENT.md)** - Step-by-step first-time deployment
- **[Deployment Summary](../DEPLOYMENT_SUMMARY.md)** - Overview of all features and deployment

### Development Guides
- **[PR Workflow](PR_WORKFLOW.md)** - Pull request validation, version checking, and build automation
- **[CI/CD Flow](CICD_FLOW.md)** - Complete automation pipeline visualization
- **[Customization Guide](CUSTOMIZATION.md)** - Adding boards, customizing features
- **[Web Flasher Setup](WEB_FLASHER_SETUP.md)** - Browser-based firmware flashing configuration

### Advanced Topics
- **[GitHub Pages Branch](GH_PAGES_BRANCH.md)** - Understanding the `gh-pages` branch strategy

## 🔄 Development Workflow

### For Contributors

1. **Making Changes**
   - Fork/clone repository
   - Create feature branch
   - Make code changes
   - Update version (if applicable)
   - Update CHANGELOG.md (if applicable)
   - Test locally: `.\build.ps1 all`

2. **Creating Pull Request**
   - Push branch to GitHub
   - Create PR to `main` branch
   - Workflow automatically validates:
     - Version incremented (if version.h exists)
     - CHANGELOG updated (if version changed)
     - All boards build successfully
     - Firmware size within limits
   - Review automated PR comments
   - Address any issues

3. **After Merge**
   - Create git tag: `git tag v1.2.3`
   - Push tag: `git push origin v1.2.3`
   - Release workflow automatically:
     - Builds all boards
     - Creates GitHub Release
     - Deploys to GitHub Pages

See **[PR Workflow Guide](PR_WORKFLOW.md)** for detailed information.

### For Users

1. **Flashing Firmware**
   - Visit project's GitHub Pages URL
   - Use web flasher (no drivers needed!)
   - Or download binaries from Releases
   - Or build from source

2. **First Boot**
   - Device creates WiFi AP
   - Connect and configure
   - Device connects to your network
   - Access config portal for settings

See **[Main README](../README.md)** for usage instructions.

## 🛠️ Build System

### Local Building

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

### CI/CD Building

**Pull Request:**
- Triggered on PR to `main`
- Validates version and builds all boards
- Posts PR comments with results

**Release:**
- Triggered on git tag push
- Builds all boards
- Creates GitHub Release
- Deploys to GitHub Pages

## 📦 Project Structure

```
esp32-multiboard-template/
├── boards/                  # Board-specific sketches
│   ├── esp32_dev/          # ESP32 DevKit V1
│   └── esp32s3_dev/        # ESP32-S3 DevKit
├── common/                 # Shared library code
│   └── src/
│       ├── logging/        # Logger system
│       ├── power/          # Power management
│       ├── config/         # Configuration storage
│       ├── wifi/           # WiFi management
│       ├── portal/         # Config portal
│       ├── mqtt/           # MQTT telemetry
│       ├── ota/            # OTA updates
│       └── modes/          # Mode controllers
├── flasher/                # Web flasher (deployed to GitHub Pages)
├── scripts/                # Build and deployment scripts
├── .github/workflows/      # GitHub Actions CI/CD
└── docs/                   # Documentation (this folder)
    ├── CUSTOMIZATION.md
    ├── PR_WORKFLOW.md
    ├── WEB_FLASHER_SETUP.md
    └── GH_PAGES_BRANCH.md
```

## 🎯 Key Features

- **Multi-board architecture** - Shared codebase for multiple ESP32 variants
- **WiFi management** - AP mode setup + client mode with fast reconnect
- **Web config portal** - Browser-based configuration
- **MQTT telemetry** - Home Assistant auto-discovery
- **Power management** - Deep sleep, wake detection, battery monitoring
- **OTA updates** - File upload and HTTP URL-based updates
- **Web flasher** - Browser-based flashing (no drivers!)
- **Automated CI/CD** - Build validation and deployment

## 📖 Documentation Overview

### [CICD_FLOW.md](CICD_FLOW.md)
Complete CI/CD pipeline visualization:
- ASCII diagrams of all workflows
- Pull request validation flow
- Release and deployment flow
- Branch strategy diagram
- Customization points
- Metrics and monitoring

### [PR_WORKFLOW.md](PR_WORKFLOW.md)
Complete guide to the pull request workflow:
- Version tracking (optional)
- Build validation (always)
- Automated PR comments
- Semantic versioning guide
- Best practices

### [CUSTOMIZATION.md](CUSTOMIZATION.md)
How to customize the template:
- Adding new boards
- Adding features
- Modifying UI
- Customizing telemetry

### [WEB_FLASHER_SETUP.md](WEB_FLASHER_SETUP.md)
Web flasher deployment guide:
- GitHub Pages setup
- Manifest generation
- Adding board variants
- Troubleshooting

### [GH_PAGES_BRANCH.md](GH_PAGES_BRANCH.md)
Understanding the branch strategy:
- Why separate branches
- What goes where
- Manual operations (rare)

## 🆘 Getting Help

1. **Check documentation** - Most questions answered here
2. **Check workflow logs** - GitHub Actions tab shows build details
3. **Check PR comments** - Automated comments provide guidance
4. **Create an issue** - For bugs or feature requests

## 🔗 Quick Links

- [Arduino CLI Documentation](https://arduino.github.io/arduino-cli/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [ESP Web Tools](https://esphome.github.io/esp-web-tools/)
- [Keep a Changelog](https://keepachangelog.com/)
- [Semantic Versioning](https://semver.org/)

## 📝 License

MIT License - See [LICENSE](../LICENSE) file for details.
