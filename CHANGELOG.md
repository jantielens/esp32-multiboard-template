# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.2] - 2025-11-09

### Fixed
- Corrected include path for main_sketch.ino.inc in board sketches

## [1.0.1] - 2025-11-09

### Fixed
- Build scripts now correctly copy `.ino.inc` files (main_sketch.ino.inc)

## [1.0.0] - 2025-11-09

### Added
- Multi-board architecture supporting ESP32 and ESP32-S3
- Shared main_sketch.ino.inc for code reuse across boards
- WiFi management with AP and client modes
- Web-based configuration portal
- MQTT telemetry with Home Assistant auto-discovery
- OTA firmware updates (file upload and HTTP URL methods)
- Power management with deep sleep support
- Browser-based web flasher using ESP Web Tools
- Complete CI/CD pipeline with GitHub Actions
  - PR validation workflow with version checking
  - Automated release workflow with firmware builds
  - GitHub Pages deployment for web flasher
- Comprehensive documentation
- Build scripts for Windows (PowerShell) and Linux/macOS (Bash)
- Logging system with configurable levels
- Configuration persistence in EEPROM

[Unreleased]: https://github.com/jantielens/esp32-multiboard-template/compare/v1.0.2...HEAD
[1.0.2]: https://github.com/jantielens/esp32-multiboard-template/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/jantielens/esp32-multiboard-template/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/jantielens/esp32-multiboard-template/releases/tag/v1.0.0
