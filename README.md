# ESP32 Multi-Board Template Web Flasher

Flash ESP32 firmware directly from your browser using [ESP Web Tools](https://esphome.github.io/esp-web-tools/).

## ⚠️ Important: Auto-Generated Files

**DO NOT manually create or edit `manifest_*.json` files in this directory.**

These files are **auto-generated** during the release workflow by `scripts/generate_manifests.sh` and are:
- Created when you push a git tag (e.g., `v1.0.0`)
- Generated from board configurations in `boards/*/board.json`
- Deployed to the `gh-pages` branch (not committed to `main`)

If you need to add a new board, see [CUSTOMIZATION.md](../docs/CUSTOMIZATION.md).

---

## 🌐 Live Flasher

Visit: **[YOUR-GITHUB-USERNAME.github.io/esp32-multiboard-template/flasher/](https://YOUR-GITHUB-USERNAME.github.io/esp32-multiboard-template/flasher/)**

## 📋 How It Works

The flasher uses the Web Serial API to flash ESP32 devices directly from your browser. No software installation required!

### Supported Boards

- **ESP32 DevKit V1** - Standard ESP32 development board
- **ESP32-S3 DevKit** - ESP32-S3 with USB OTG and more RAM/Flash

## 🔧 For Developers

### Manifest Files Are Auto-Generated

**DO NOT manually create manifest files.** They are automatically generated during the release workflow:

1. You create a board in `boards/your_board/` with `board.json`
2. You push a git tag (e.g., `v1.0.0`)
3. GitHub Actions runs `scripts/generate_manifests.sh`
4. Manifest files are created and deployed to `gh-pages` branch

**Example auto-generated manifest** (for reference only):
```json
{
  "name": "ESP32 Template for ESP32 DevKit V1",
  "version": "1.0.0",
  "new_install_prompt_erase": true,
  "builds": [{
    "chipFamily": "ESP32",
    "parts": [
      { "path": "https://..../esp32_dev-v1.0.0.bootloader.bin", "offset": 4096 },
      { "path": "https://..../esp32_dev-v1.0.0.partitions.bin", "offset": 32768 },
      { "path": "https://..../esp32_dev-v1.0.0.bin", "offset": 65536 }
    ]
  }]
}
```

See [WEB_FLASHER_SETUP.md](../docs/WEB_FLASHER_SETUP.md) for complete setup guide.

### Testing Locally

1. Build firmware:
   ```powershell
   .\build.ps1 all
   ```

2. Start a local web server:
   ```powershell
   cd flasher
   python -m http.server 8000
   ```

3. Open `http://localhost:8000` in Chrome/Edge

4. For local testing, create test manifests pointing to local build files

## 📱 Browser Support

The Web Serial API is supported in:
- ✅ Chrome 89+
- ✅ Edge 89+
- ✅ Opera 75+
- ❌ Firefox (not supported)
- ❌ Safari (not supported)

## 🚀 Deployment

### GitHub Pages

1. Enable GitHub Pages in repository settings
2. Set source to `main` branch, `/` (root)
3. Flasher will be available at: `https://YOUR-USERNAME.github.io/esp32-multiboard-template/flasher/`

### Manual Deployment

Copy the `flasher/` directory to any static web host. Ensure:
- `manifest_*.json` files point to correct GitHub release URLs
- Files are served with correct MIME types
- HTTPS is enabled (required for Web Serial API)

## 📚 References

- [ESP Web Tools Documentation](https://esphome.github.io/esp-web-tools/)
- [Web Serial API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API)
- [ESP32 Flash Layout](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html)
