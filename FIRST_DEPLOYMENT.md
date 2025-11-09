# First Deployment Checklist

Use this checklist when deploying the web flasher for the first time.

## ☑️ Pre-Deployment Setup

### 1. GitHub Repository Setup
- [ ] Created new repository from template ("Use this template" button)
- [ ] Cloned your new repository locally
- [ ] Repository configured with your project name

### 2. GitHub Pages Configuration
- [ ] Go to repository Settings → Pages
- [ ] Source: "Deploy from a branch"
- [ ] Branch: `gh-pages` (will be auto-created on first tag)
- [ ] Folder: `/ (root)`
- [ ] Click "Save"

**Note:** The `gh-pages` branch doesn't exist yet - it will be created automatically by the workflow.

### 3. Workflow Permissions
- [ ] Go to repository Settings → Actions → General
- [ ] Scroll to "Workflow permissions"
- [ ] Select "Read and write permissions"
- [ ] Check "Allow GitHub Actions to create and approve pull requests"
- [ ] Click "Save"

**This allows the workflows to:**
- Create PR comments (build status, version checks)
- Create the `gh-pages` branch
- Push firmware binaries
- Create GitHub Releases

### 3a. Optional: Test PR Workflow
Before your first release, you can test the PR validation workflow:

- [ ] Create a feature branch: `git checkout -b test-pr`
- [ ] Make a small change (e.g., add a comment to a file)
- [ ] Create optional `common/src/version.h` file (for version tracking):
  ```cpp
  #ifndef VERSION_H
  #define VERSION_H
  
  #define FIRMWARE_VERSION "1.0.0"
  
  #endif // VERSION_H
  ```
- [ ] Create optional `CHANGELOG.md` file:
  ```markdown
  # Changelog
  
  ## [1.0.0] - 2025-11-09
  ### Added
  - Initial release
  ```
- [ ] Push branch and create a PR
- [ ] Check PR for automated comments (version check + build summary)
- [ ] Verify workflow succeeds in Actions tab
- [ ] Close/merge the test PR

**Note:** Version and CHANGELOG files are optional. See [Developer Guide](docs/DEVELOPER_GUIDE.md) for details.

## â˜ First Release

### 4. Prepare Release
- [ ] All code committed to `main` branch
- [ ] Build tested locally: `.\build.ps1 all` (both boards succeed)
- [ ] `.gitignore` configured (no binaries in main)
- [ ] README.md updated with project info

## ☑️ First Release

### 5. Create Git Tag
```powershell
# Create tag (semantic versioning)
git tag v1.0.0

# Push tag to trigger workflow
git push origin v1.0.0
```

### 6. Monitor Workflow
- [ ] Go to repository "Actions" tab
- [ ] Watch "Release and Deploy" workflow
- [ ] Verify all steps complete successfully:
  - [ ] Build ESP32 board
  - [ ] Build ESP32-S3 board
  - [ ] Create GitHub Release
  - [ ] Generate manifests
  - [ ] Commit to gh-pages branch
  - [ ] Deploy to GitHub Pages

**Expected duration:** 3-5 minutes

### 7. Verify Deployment
- [ ] Check GitHub Release created (with 12 binary files)
- [ ] Verify `gh-pages` branch created: `git fetch origin && git branch -a | Select-String pages`
- [ ] Wait 1-2 minutes for GitHub Pages to deploy
- [ ] Visit flasher URL: `https://<username>.github.io/<repo-name>/`
- [ ] Verify both boards appear in dropdown
- [ ] Click "Connect" button (should request serial port)

## ☑️ Post-Deployment Verification

### 8. Test Web Flasher
- [ ] Connect ESP32 device via USB
- [ ] Select correct board (ESP32 DevKit V1 or ESP32-S3 DevKit)
- [ ] Click "Connect" → Select serial port
- [ ] Click "Install" → Confirm install
- [ ] Wait for flash to complete (~1 minute)
- [ ] Verify device reboots successfully
- [ ] Check serial monitor for boot messages

### 9. Test Firmware
- [ ] Device creates AP: `esp32-XXXXXX`
- [ ] Connect to AP and visit `http://192.168.4.1`
- [ ] Config portal loads successfully
- [ ] Configure WiFi and save
- [ ] Device connects to WiFi
- [ ] Access config portal at device IP
- [ ] Test OTA page: `http://<device-ip>/ota`

### 10. Test OTA Update
- [ ] Make a small code change (e.g., update a log message)
- [ ] Create new tag: `v1.0.1`
- [ ] Push tag: `git push origin v1.0.1`
- [ ] Wait for workflow to complete
- [ ] Use OTA URL method to update device:
  - Visit `http://<device-ip>/ota`
  - Enter URL: `https://<username>.github.io/<repo>/firmware/v1.0.1/esp32_dev.ino.bin`
  - Click "Update"
- [ ] Device updates and reboots
- [ ] Verify new code running

## 🐛 Troubleshooting

### Workflow Fails: "Permission denied"
**Fix:** Update workflow permissions (Step 3 above)

### Workflow Fails: "gh-pages branch not found"
**Expected on first run.** The workflow creates it automatically.

### GitHub Pages 404 Error
- Check Settings → Pages is enabled
- Verify `gh-pages` branch exists: `git branch -a`
- Wait 2-5 minutes after first deployment
- Check Actions → pages-build-deployment workflow

### Flasher Shows "No firmware files"
- Check `gh-pages` branch has `flasher/firmware/v1.0.0/` directory
- Verify manifests updated with correct URLs
- Check browser console for errors (F12)
- Try hard refresh: Ctrl+F5

### Can't Connect to Device in Web Flasher
- Use Chrome, Edge, or Opera (not Firefox/Safari)
- Ensure USB drivers installed (CP210x or CH340)
- Check device shows in Device Manager (Windows) or `ls /dev/tty*` (Linux)
- Try different USB cable/port

## 📝 Notes

**Main Branch Should NOT Have:**
- Firmware `.bin` files
- `flasher/firmware/` directory
- `flasher/latest.json` file

**gh-pages Branch Should Have:**
- All flasher files (HTML, JS, CSS)
- All manifests
- `firmware/` directory with all versions
- `latest.json` metadata

**On Every New Tag:**
- Workflow builds fresh binaries
- New `firmware/vX.Y.Z/` directory created
- Manifests updated to point to new version
- `latest.json` updated
- GitHub Pages auto-deploys

## ✅ Success Criteria

- [ ] Workflow completes without errors
- [ ] GitHub Release created with 12 files
- [ ] `gh-pages` branch exists with flasher + firmware
- [ ] GitHub Pages serves flasher at public URL
- [ ] Web flasher loads in browser
- [ ] Both boards flashable via web
- [ ] Flashed device boots and works
- [ ] OTA updates functional

**When all checked: Deployment successful! 🎉**

## 🔄 Subsequent Releases

For future releases, only steps 5-7 are needed:
1. Make code changes
2. Commit to main
3. Create new tag (increment version)
4. Push tag
5. Workflow auto-deploys

**No manual intervention required after first setup!**
