# ESP32 Multi-Board Template Web Flasher

Flash ESP32 firmware directly from your browser using [ESP Web Tools](https://esphome.github.io/esp-web-tools/).

## 🌐 Live Flasher

Visit: **[YOUR-GITHUB-USERNAME.github.io/esp32-multiboard-template/flasher/](https://YOUR-GITHUB-USERNAME.github.io/esp32-multiboard-template/flasher/)**

## 📋 How It Works

The flasher uses the Web Serial API to flash ESP32 devices directly from your browser. No software installation required!

### Supported Boards

- **ESP32 DevKit V1** - Standard ESP32 development board
- **ESP32-S3 DevKit** - ESP32-S3 with USB OTG and more RAM/Flash

## 🔧 For Developers

### Creating a Release with Flasher

When you create a GitHub release, include these binaries for each board:

```
{board}-v{version}.bin              # Main firmware
{board}-v{version}.bootloader.bin   # Bootloader
{board}-v{version}.partitions.bin   # Partition table
```

Example for v1.0.0:
```
esp32_dev-v1.0.0.bin
esp32_dev-v1.0.0.bootloader.bin
esp32_dev-v1.0.0.partitions.bin
esp32s3_dev-v1.0.0.bin
esp32s3_dev-v1.0.0.bootloader.bin
esp32s3_dev-v1.0.0.partitions.bin
```

### Generating Manifests

Create manifest files for ESP Web Tools:

**manifest_esp32_dev.json:**
```json
{
  "name": "ESP32 Template for ESP32 DevKit V1",
  "version": "1.0.0",
  "new_install_prompt_erase": true,
  "builds": [
    {
      "chipFamily": "ESP32",
      "parts": [
        {
          "path": "https://github.com/YOUR-USERNAME/esp32-multiboard-template/releases/download/v1.0.0/esp32_dev-v1.0.0.bootloader.bin",
          "offset": 4096
        },
        {
          "path": "https://github.com/YOUR-USERNAME/esp32-multiboard-template/releases/download/v1.0.0/esp32_dev-v1.0.0.partitions.bin",
          "offset": 32768
        },
        {
          "path": "https://github.com/YOUR-USERNAME/esp32-multiboard-template/releases/download/v1.0.0/esp32_dev-v1.0.0.bin",
          "offset": 65536
        }
      ]
    }
  ]
}
```

**manifest_esp32s3_dev.json:**
```json
{
  "name": "ESP32 Template for ESP32-S3 DevKit",
  "version": "1.0.0",
  "new_install_prompt_erase": true,
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "parts": [
        {
          "path": "https://github.com/YOUR-USERNAME/esp32-multiboard-template/releases/download/v1.0.0/esp32s3_dev-v1.0.0.bootloader.bin",
          "offset": 0
        },
        {
          "path": "https://github.com/YOUR-USERNAME/esp32-multiboard-template/releases/download/v1.0.0/esp32s3_dev-v1.0.0.partitions.bin",
          "offset": 32768
        },
        {
          "path": "https://github.com/YOUR-USERNAME/esp32-multiboard-template/releases/download/v1.0.0/esp32s3_dev-v1.0.0.bin",
          "offset": 65536
        }
      ]
    }
  ]
}
```

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
