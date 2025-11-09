# CI/CD Flow - Complete Automation Pipeline

This document visualizes the complete CI/CD automation pipeline for the ESP32 Multi-Board Template.

## Overview

The template uses **three GitHub Actions workflows** to automate development, validation, and deployment:

1. **`build.yml`** - Pull Request validation
2. **`release.yml`** - Release automation
3. **`deploy-pages.yml`** - GitHub Pages deployment

## 🔄 Complete CI/CD Flow Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        DEVELOPMENT PHASE                         │
└─────────────────────────────────────────────────────────────────┘

Developer makes changes locally
         │
         ├─── Creates feature branch
         ├─── Modifies code
         ├─── Updates version.h (optional)
         ├─── Updates CHANGELOG.md (optional)
         ├─── Tests locally: .\build.ps1 all
         └─── Pushes to GitHub
                    ↓

┌─────────────────────────────────────────────────────────────────┐
│                    PULL REQUEST VALIDATION                       │
│                      (build.yml workflow)                        │
└─────────────────────────────────────────────────────────────────┘

Creates Pull Request → main
         ↓
    build.yml triggers
         ↓
┌────────────────────────────────┐
│   JOB 1: Version Check         │
│   (if PR to main)              │
├────────────────────────────────┤
│ 1. Checkout PR branch          │
│ 2. Checkout main branch        │
│ 3. Compare version.h files     │
│ 4. Check CHANGELOG.md entry    │
│ 5. Post PR comment:            │
│    ✅ Version incremented      │
│    ⚠️  Version unchanged       │
│    ❌ Version decreased        │
└────────────────────────────────┘
         ↓
┌────────────────────────────────┐
│   JOB 2: Prepare Matrix        │
├────────────────────────────────┤
│ Set boards to build:           │
│ - esp32_dev                    │
│ - esp32s3_dev                  │
└────────────────────────────────┘
         ↓
┌────────────────────────────────────────────────────────┐
│   JOB 3: Build (runs in parallel for each board)      │
├────────────────────────────────────────────────────────┤
│ Matrix:                                                │
│ ┌──────────────┐              ┌──────────────┐       │
│ │ esp32_dev    │              │ esp32s3_dev  │       │
│ ├──────────────┤              ├──────────────┤       │
│ │ 1. Checkout  │              │ 1. Checkout  │       │
│ │ 2. Cache CLI │              │ 2. Cache CLI │       │
│ │ 3. Install   │              │ 3. Install   │
│ │    Arduino   │              │    Arduino   │       │
│ │    CLI       │              │    CLI       │       │
│ │ 4. Run       │              │ 4. Run       │       │
│ │    build.sh  │              │    build.sh  │       │
│ │    (auto     │              │    (auto     │       │
│ │    installs  │              │    installs  │       │
│ │    cores &   │              │    cores &   │       │
│ │    libs)     │              │    libs)     │       │
│ │ 5. Check     │              │ 5. Check     │       │
│ │    size      │              │    size      │       │
│ │    <1.5MB    │              │    <1.5MB    │       │
│ │ 6. Upload    │              │ 6. Upload    │       │
│ │    artifact  │              │    artifact  │       │
│ └──────────────┘              └──────────────┘       │
└────────────────────────────────────────────────────────┘
         ↓
┌────────────────────────────────┐
│   JOB 4: Comment Summary       │
│   (if PR)                      │
├────────────────────────────────┤
│ 1. Download all artifacts      │
│ 2. Calculate sizes             │
│ 3. Generate table:             │
│                                │
│    | Board    | Size | % Max | │
│    |----------|------|-------| │
│    | esp32    | 1.1M | 85%   | │
│    | esp32s3  | 1.0M | 84%   | │
│                                │
│ 4. Post/update PR comment      │
└────────────────────────────────┘
         ↓
Developer reviews PR comments
Developer addresses issues (if any)
Developer merges PR
         ↓

┌─────────────────────────────────────────────────────────────────┐
│                         RELEASE PHASE                            │
│                    (release.yml workflow)                        │
└─────────────────────────────────────────────────────────────────┘

Developer creates git tag
         │
         ├─── git tag v1.2.3
         └─── git push origin v1.2.3
                    ↓
    release.yml triggers (on tag push)
         ↓
┌────────────────────────────────┐
│   JOB 1: Validate Tag          │
├────────────────────────────────┤
│ 1. Extract version from tag    │
│ 2. Read version.h file         │
│ 3. Verify match:               │
│    tag: v1.2.3                 │
│    version.h: "1.2.3"          │
│ 4. Fail if mismatch            │
└────────────────────────────────┘
         ↓
┌────────────────────────────────────────────────────────┐
│   JOB 2: Build All Boards (parallel)                   │
├────────────────────────────────────────────────────────┤
│ For each board:                                        │
│ 1. Build firmware                                      │
│ 2. Build bootloader                                    │
│ 3. Build partitions                                    │
│ 4. Upload artifacts                                    │
│                                                         │
│ Outputs (per board):                                   │
│ - esp32_dev.ino.bin           (main firmware)         │
│ - esp32_dev.ino.bootloader.bin                        │
│ - esp32_dev.ino.partitions.bin                        │
└────────────────────────────────────────────────────────┘
         ↓
┌────────────────────────────────┐
│   JOB 3: Create Release        │
├────────────────────────────────┤
│ 1. Download all artifacts      │
│ 2. Extract CHANGELOG section   │
│ 3. Create GitHub Release:      │
│    - Title: v1.2.3             │
│    - Body: CHANGELOG excerpt   │
│    - Assets: 12 binary files   │
│      (2 boards × 6 files each) │
└────────────────────────────────┘
         ↓
┌────────────────────────────────┐
│   JOB 4: Generate Manifests    │
├────────────────────────────────┤
│ 1. Run generate_manifests.sh   │
│    - Creates ESP Web Tools     │
│      manifest files            │
│    - Points to GitHub Pages    │
│      URLs:                     │
│      https://user.github.io/   │
│      repo/firmware/v1.2.3/     │
│      esp32_dev.ino.bin         │
│                                │
│ 2. Run generate_latest_json.sh │
│    - Creates metadata JSON     │
│    - Lists all firmware URLs   │
│    - Includes version, date    │
└────────────────────────────────┘
         ↓
┌────────────────────────────────┐
│   JOB 5: Deploy to gh-pages    │
├────────────────────────────────┤
│ 1. Checkout gh-pages branch    │
│    (create if doesn't exist)   │
│                                │
│ 2. Copy files:                 │
│    flasher/                    │
│    ├── index.html              │
│    ├── app.js                  │
│    ├── styles.css              │
│    ├── manifest_esp32_dev.json │
│    ├── manifest_esp32s3_dev... │
│    ├── latest.json             │
│    └── firmware/               │
│        └── v1.2.3/             │
│            ├── esp32_dev...bin │
│            ├── esp32_dev...    │
│            │   bootloader.bin  │
│            ├── esp32_dev...    │
│            │   partitions.bin  │
│            ├── esp32s3_dev...  │
│            └── ...             │
│                                │
│ 3. Commit and push to          │
│    gh-pages branch             │
└────────────────────────────────┘
         ↓
┌─────────────────────────────────────────────────────────────────┐
│                    GITHUB PAGES DEPLOYMENT                       │
│                  (deploy-pages.yml workflow)                     │
└─────────────────────────────────────────────────────────────────┘

gh-pages branch updated
         ↓
    deploy-pages.yml triggers
         ↓
┌────────────────────────────────┐
│   JOB 1: Deploy to Pages       │
├────────────────────────────────┤
│ 1. Configure GitHub Pages      │
│ 2. Upload artifact (flasher/)  │
│ 3. Deploy to GitHub Pages      │
│ 4. Site live at:               │
│    https://user.github.io/repo │
└────────────────────────────────┘
         ↓

┌─────────────────────────────────────────────────────────────────┐
│                         USER ACCESS                              │
└─────────────────────────────────────────────────────────────────┘

User visits flasher site
         │
         ├─── Opens browser (Chrome/Edge/Opera)
         ├─── Visits https://user.github.io/repo/
         ├─── Selects board (ESP32 or ESP32-S3)
         ├─── Clicks "Connect" → Selects serial port
         ├─── Clicks "Install" → Firmware downloads
         │                    ↓
         │    Firmware flashes from GitHub Pages URL
         │    (no CORS issues, proper headers)
         │                    ↓
         └─── Device reboots with new firmware
                    ↓
              Success! 🎉
```

## 📊 Workflow Summary

### Pull Request Workflow (build.yml)

| Phase | Duration | Actions |
|-------|----------|---------|
| Version Check | ~30 sec | Compare versions, check CHANGELOG, post comment |
| Build Matrix Setup | ~5 sec | Configure boards to build |
| Build (parallel) | ~2 min | Compile 2 boards simultaneously |
| Comment Summary | ~10 sec | Generate table, update PR comment |
| **Total** | **~3 min** | **Validates PR before merge** |

**Triggers:**
- Pull request to `main` branch
- Excludes: docs, markdown, flasher files
- Manual trigger available

**Outputs:**
- PR comments (version status, build summary)
- Build artifacts (downloadable for 30 days)

### Release Workflow (release.yml)

| Phase | Duration | Actions |
|-------|----------|---------|
| Validate Tag | ~10 sec | Check tag matches version.h |
| Build All Boards | ~3 min | Build 2 boards with bootloaders/partitions |
| Create Release | ~30 sec | GitHub Release with 12 binaries |
| Generate Manifests | ~10 sec | Create ESP Web Tools manifests + metadata |
| Deploy to gh-pages | ~20 sec | Commit firmware to gh-pages branch |
| **Total** | **~4 min** | **From tag to deployed flasher** |

**Triggers:**
- Git tag push (format: `v*.*.*`)

**Outputs:**
- GitHub Release with binaries
- Updated gh-pages branch
- Web flasher available immediately

### Pages Deployment (deploy-pages.yml)

| Phase | Duration | Actions |
|-------|----------|---------|
| Configure Pages | ~10 sec | Setup GitHub Pages environment |
| Deploy | ~30 sec | Upload and publish to GitHub Pages |
| **Total** | **~40 sec** | **Flasher site updated** |

**Triggers:**
- Push to `gh-pages` branch
- Automatic after release.yml

**Outputs:**
- Live web flasher at GitHub Pages URL
- Accessible worldwide via HTTPS

## 🎯 Branch Strategy

```
┌────────────────────────────────────────────────────────┐
│                      main branch                        │
│  (Clean code, NO binaries)                             │
├────────────────────────────────────────────────────────┤
│ - Source code (.cpp, .h, .ino)                         │
│ - Build scripts (build.ps1, build.sh)                  │
│ - Documentation (.md files)                            │
│ - Workflows (.github/workflows/)                       │
│ - Flasher templates (HTML, JS, CSS)                    │
│ - Manifest templates (.json)                           │
│                                                         │
│ ❌ NO firmware binaries (.bin files)                   │
│ ❌ NO firmware/ directory                              │
│ ❌ NO latest.json                                      │
└────────────────────────────────────────────────────────┘
                           ↓
                    (on git tag push)
                           ↓
┌────────────────────────────────────────────────────────┐
│                   gh-pages branch                       │
│  (Flasher site + firmware binaries)                    │
├────────────────────────────────────────────────────────┤
│ flasher/                                               │
│ ├── index.html              (from main)                │
│ ├── app.js                  (from main)                │
│ ├── styles.css              (from main)                │
│ ├── manifest_esp32_dev.json (GENERATED)                │
│ ├── manifest_esp32s3_dev... (GENERATED)                │
│ ├── latest.json             (GENERATED)                │
│ └── firmware/               (GENERATED)                │
│     ├── v1.0.0/                                        │
│     │   ├── esp32_dev.ino.bin                          │
│     │   ├── esp32_dev.ino.bootloader.bin               │
│     │   ├── esp32_dev.ino.partitions.bin               │
│     │   └── ...                                        │
│     ├── v1.1.0/                                        │
│     │   └── ...                                        │
│     └── v1.2.3/  (latest)                              │
│         └── ...                                        │
│                                                         │
│ ✅ Contains all firmware releases                      │
│ ✅ Deployed to GitHub Pages                            │
│ ✅ Served with proper CORS headers                     │
└────────────────────────────────────────────────────────┘
                           ↓
                    (GitHub Pages)
                           ↓
        https://username.github.io/repo/
```

## 🔐 Required Permissions

### Repository Settings

**Settings → Actions → General → Workflow permissions:**
- ✅ Read and write permissions
- ✅ Allow GitHub Actions to create and approve pull requests

**Settings → Pages:**
- ✅ Source: Deploy from a branch
- ✅ Branch: `gh-pages`
- ✅ Folder: `/ (root)`

### Why These Permissions?

| Permission | Used By | Purpose |
|------------|---------|---------|
| Read repository | All workflows | Checkout code |
| Write repository | release.yml | Create/push to gh-pages branch |
| Read/write PRs | build.yml | Post/update PR comments |
| Create releases | release.yml | Create GitHub Release |
| Deploy Pages | deploy-pages.yml | Publish to GitHub Pages |

## 🎨 Customization Points

### Adding a New Board

**1. Create board files** in `boards/new_board/`:
- `board.json` - Board metadata with name and FQBN
- `board_config.h` - Hardware constants
- `new_board.ino` - Minimal sketch

**2. Update build matrix** in `.github/workflows/build.yml`:
```yaml
prepare:
  steps:
    - name: Set build matrix
      run: |
        echo 'matrix=["esp32_dev","esp32s3_dev","new_board"]' >> $GITHUB_OUTPUT
```

**3. Update release workflow** in `.github/workflows/release.yml`:
```yaml
build:
  strategy:
    matrix:
      board: [esp32_dev, esp32s3_dev, new_board]
```

**4. Update manifest script** in `scripts/generate_manifests.sh`:
```bash
# Add new board section
cat > "${FLASHER_DIR}/manifest_new_board.json" << EOF
{
  "name": "New Board",
  "builds": [...]
}
EOF
```

**5. Update flasher UI** in `flasher/app.js`:
```javascript
const devices = [
  { id: 'esp32_dev', name: 'ESP32 DevKit V1', ... },
  { id: 'esp32s3_dev', name: 'ESP32-S3 DevKit', ... },
  { id: 'new_board', name: 'New Board Name', ... }
];
```

**Note:** Local build scripts (build.ps1/build.sh) automatically discover boards from board.json files - no manual editing needed for local builds!
```

### Changing Build Triggers

**Skip more file patterns:**
```yaml
on:
  pull_request:
    paths-ignore:
      - '**.md'
      - 'docs/**'
      - 'flasher/**'
      - 'examples/**'  # Add this
```

**Change target branches:**
```yaml
on:
  pull_request:
    branches:
      - main
      - develop  # Add this
```

### Adjusting Firmware Size Limits

In `build.yml`, update the size check:
```yaml
- name: Get firmware size
  run: |
    MAX_SIZE=2097152  # Change to 2MB
    # ... rest of check
```

## 📈 Metrics & Monitoring

### What to Monitor

**In GitHub Actions tab:**
- ✅ Workflow run times (should be <5 minutes)
- ✅ Build success rate (should be >95%)
- ✅ Cache hit rate (faster builds)
- ⚠️ Failed builds (investigate immediately)

**In Releases:**
- ✅ Firmware sizes (track growth over time)
- ✅ Release frequency (regular releases = healthy project)
- ✅ Download counts (user engagement)

**In Pull Requests:**
- ✅ PR comment quality (helpful feedback?)
- ✅ Version increment compliance
- ✅ Build time consistency

### Optimization Tips

**Speed up builds:**
- Cache Arduino CLI and libraries (already done)
- Use matrix builds for parallelization (already done)
- Only run on code changes (already done)

**Reduce storage:**
- Limit artifact retention (currently 30 days)
- Clean up old releases manually
- Remove unused build outputs

**Improve reliability:**
- Pin dependency versions (Arduino CLI 1.3.1, ESP32 3.3.2)
- Use stable GitHub Actions (actions/checkout@v4)
- Add retry logic for flaky steps

## 🐛 Troubleshooting

### Build Workflow Issues

**Symptom:** Version check fails to find version.h
- **Cause:** File doesn't exist or wrong path
- **Fix:** Create `common/src/version.h` or check path

**Symptom:** Build fails with "core not found"
- **Cause:** Cache corruption or wrong version
- **Fix:** Clear cache, rebuild

**Symptom:** PR comment not posted
- **Cause:** Missing workflow permissions
- **Fix:** Enable "Read and write permissions" in Settings

### Release Workflow Issues

**Symptom:** Tag validation fails
- **Cause:** Tag doesn't match version.h
- **Fix:** Ensure `git tag v1.2.3` matches `#define FIRMWARE_VERSION "1.2.3"`

**Symptom:** gh-pages deployment fails
- **Cause:** Missing write permissions
- **Fix:** Check workflow permissions in Settings

**Symptom:** Firmware too large
- **Cause:** Exceeded partition size
- **Fix:** Optimize code, remove unused libraries

### Pages Deployment Issues

**Symptom:** 404 error on flasher site
- **Cause:** GitHub Pages not enabled
- **Fix:** Enable in Settings → Pages

**Symptom:** Stale firmware on flasher
- **Cause:** Workflow didn't run or failed
- **Fix:** Check Actions logs, re-trigger workflow

**Symptom:** Web Serial not working
- **Cause:** Wrong browser or HTTPS issue
- **Fix:** Use Chrome/Edge, ensure HTTPS enabled

## 🎓 Learning Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [ESP Web Tools Documentation](https://esphome.github.io/esp-web-tools/)
- [Semantic Versioning Specification](https://semver.org/)
- [Keep a Changelog Format](https://keepachangelog.com/)

## 📝 Related Documentation

- [PR_WORKFLOW.md](PR_WORKFLOW.md) - Detailed PR validation guide
- [WEB_FLASHER_SETUP.md](WEB_FLASHER_SETUP.md) - Web flasher configuration
- [FIRST_DEPLOYMENT.md](../FIRST_DEPLOYMENT.md) - First-time setup checklist
- [DEPLOYMENT_SUMMARY.md](../DEPLOYMENT_SUMMARY.md) - Feature overview
