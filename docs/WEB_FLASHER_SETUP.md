# Web Flasher Setup Guide

This guide explains how to set up the web flasher for your fork of the ESP32 Multi-Board Template.

## 🎯 Overview

The web flasher allows users to flash firmware directly from their browser without installing Arduino IDE or drivers. It uses:
- **GitHub Actions** to build firmware on release
- **GitHub Pages** to host the flasher site and binaries
- **ESP Web Tools** for browser-based flashing via Web Serial API

**Note:** This guide assumes you've created your own repository using the "Use this template" button on GitHub.

## 📋 Architecture

### Branch Strategy

- **`main` branch** - Clean code repository (no binaries)
- **`gh-pages` branch** - Flasher website + firmware binaries (auto-created by workflow)

### Workflow

```
1. User creates git tag (v1.0.0)
2. GitHub Action builds all boards
3. Action generates manifests with URLs pointing to GitHub Pages
4. Action commits binaries + manifests to gh-pages branch
5. GitHub Pages auto-deploys from gh-pages branch
6. Flasher is live at: https://YOUR-USERNAME.github.io/esp32-multiboard-template/
```

## 🚀 Setup Instructions

### Step 1: Enable GitHub Pages

1. Go to your repository **Settings** → **Pages**
2. Under **Source**, select:
   - **Source**: Deploy from a branch
   - **Branch**: `gh-pages`
   - **Folder**: `/ (root)`
3. Click **Save**

GitHub will create the `gh-pages` branch automatically on first release.

### Step 2: Update Flasher URLs (Optional)

The workflow automatically uses your GitHub username and repository name. If you forked with a different name, the URLs will update automatically.

To verify, check `flasher/index.html` line 30:
```html
<a href="https://github.com/YOUR-USERNAME/esp32-multiboard-template" ...>
```

Update if needed to match your repository URL.

### Step 3: Create Your First Release

1. **Update version** (if desired):
   ```bash
   # Edit common/src/version.h
   #define FIRMWARE_VERSION "1.0.0"
   ```

2. **Commit and push to main**:
   ```bash
   git add .
   git commit -m "Prepare v1.0.0 release"
   git push origin main
   ```

3. **Create and push a git tag**:
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```

4. **Watch the workflow**:
   - Go to **Actions** tab in GitHub
   - Watch `Create Release` workflow build firmware
   - This will take ~5-10 minutes

5. **Verify deployment**:
   - Check **Releases** page for new release with binaries
   - Wait for `Deploy Flasher to GitHub Pages` workflow to complete
   - Visit: `https://YOUR-USERNAME.github.io/esp32-multiboard-template/`

## 📂 What Gets Deployed

The `gh-pages` branch contains:

```
flasher/
  index.html                    # Flasher UI
  app.js                        # Device selection logic
  styles.css                    # Styling
  README.md                     # Flasher documentation
  manifest_esp32_dev.json       # ESP32 flash instructions (updated per release)
  manifest_esp32s3_dev.json     # ESP32-S3 flash instructions (updated per release)
  latest.json                   # Metadata about latest release
  firmware/
    v1.0.0/                     # Versioned firmware binaries
      esp32_dev-v1.0.0.bin
      esp32_dev-v1.0.0.bootloader.bin
      esp32_dev-v1.0.0.partitions.bin
      esp32s3_dev-v1.0.0.bin
      esp32s3_dev-v1.0.0.bootloader.bin
      esp32s3_dev-v1.0.0.partitions.bin
    v1.1.0/                     # Next release...
      ...
```

## 🔧 Adding New Boards

When you add a new board to the template:

**Local Build Setup (automatic):**
1. Create `boards/your_new_board/board.json` with name and FQBN
2. Build scripts automatically discover it - no editing needed!

**CI/CD Setup (manual):**
1. **Update board list** in `scripts/generate_manifests.sh`:
   ```bash
   NAMES[your_new_board]="Your New Board Name"
   CHIP_FAMILIES[your_new_board]="ESP32"  # or "ESP32-S3"
   ```

2. **Update board list** in `scripts/generate_latest_json.sh`:
   ```bash
   NAMES[your_new_board]="Your New Board Name"
   ```

3. **Update workflow** in `.github/workflows/release.yml`:
   ```yaml
   strategy:
     matrix:
       board: [esp32_dev, esp32s3_dev, your_new_board]
   ```

4. **Update workflow** in `.github/workflows/build.yml`:
   ```yaml
   prepare:
     steps:
       - name: Set build matrix
         run: |
           echo 'matrix=["esp32_dev","esp32s3_dev","your_new_board"]' >> $GITHUB_OUTPUT
   ```

5. **Update flasher UI** in `flasher/app.js`:
   ```javascript
   const BOARDS = {
     // ... existing boards
     your_new_board: {
       name: 'Your New Board Name',
       icon: '🔧',
       description: 'Brief description'
     }
   };
   ```

6. **Commit and create a new release** - workflow will handle the rest!

**Note:** The CI/CD workflows still use hardcoded board lists for matrix builds. This is intentional to maintain explicit control over which boards are built in GitHub Actions.

## 🐛 Troubleshooting

### Flasher Shows 404 Errors

**Cause**: Manifests point to GitHub Pages URLs, but files haven't been deployed yet.

**Fix**: 
- Wait for `Deploy Flasher to GitHub Pages` workflow to complete
- Check that `gh-pages` branch exists and has `flasher/firmware/` directory
- Verify GitHub Pages is enabled in repository settings

### Flashing Fails with CORS Error

**Cause**: Trying to flash from `file://` URL or wrong domain.

**Fix**: 
- Always access via GitHub Pages URL: `https://YOUR-USERNAME.github.io/esp32-multiboard-template/`
- Never open `index.html` directly from filesystem

### Manifest URLs Point to Wrong Repository

**Cause**: Scripts use environment variables that may be incorrect.

**Fix**: 
- Workflow automatically uses `${{ github.repository_owner }}` and `${{ github.event.repository.name }}`
- If you renamed the repo, update `env` section in `.github/workflows/release.yml`

### Old Firmware Versions Missing

**Cause**: Previous releases weren't deployed to `gh-pages` branch.

**Solution**: 
- Old releases in `Releases` page still have downloadable binaries
- Flasher only shows latest version (by design)
- To keep all versions in flasher, don't delete `flasher/firmware/*` directories

## 📚 How It Works

### 1. Manifest Generation

Scripts generate ESP Web Tools manifests pointing to GitHub Pages:

```json
{
  "name": "ESP32 Template for ESP32 DevKit V1",
  "version": "1.0.0",
  "builds": [{
    "chipFamily": "ESP32",
    "parts": [
      {
        "path": "https://YOUR-USERNAME.github.io/esp32-multiboard-template/firmware/v1.0.0/esp32_dev-v1.0.0.bootloader.bin",
        "offset": 4096
      },
      {
        "path": "https://YOUR-USERNAME.github.io/esp32-multiboard-template/firmware/v1.0.0/esp32_dev-v1.0.0.partitions.bin",
        "offset": 32768
      },
      {
        "path": "https://YOUR-USERNAME.github.io/esp32-multiboard-template/firmware/v1.0.0/esp32_dev-v1.0.0.bin",
        "offset": 65536
      }
    ]
  }]
}
```

### 2. CORS Compliance

GitHub Pages serves files with proper CORS headers, allowing Web Serial API to download and flash them.

### 3. Version Management

Each release creates a versioned `flasher/firmware/v{VERSION}/` directory. Manifests always point to the correct version URLs.

## 🎉 Success!

Once set up, your workflow is:

1. **Develop** on `main` branch
2. **Create tag** when ready to release
3. **GitHub Actions** handles the rest:
   - ✅ Builds firmware
   - ✅ Creates GitHub Release
   - ✅ Generates manifests
   - ✅ Deploys to GitHub Pages
   - ✅ Flasher is live automatically!

Users can flash firmware directly from `https://YOUR-USERNAME.github.io/esp32-multiboard-template/` without any installation! 🚀
