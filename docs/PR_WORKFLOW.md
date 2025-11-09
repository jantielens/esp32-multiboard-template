# Pull Request Workflow

The `build.yml` workflow automatically validates pull requests to ensure code quality and proper versioning.

## What Gets Checked

### 1. Version Tracking (Optional)

If you have a `common/src/version.h` file, the workflow checks:

- ✅ **Version incremented** - Ensures version number increases (semantic versioning)
- ✅ **CHANGELOG updated** - Verifies CHANGELOG.md has an entry for the new version
- ⚠️ **Version unchanged** - Warns if version not incremented (not a blocker)
- ❌ **Version decreased** - Warns if version went down (unusual)

**Example `common/src/version.h`:**
```cpp
#ifndef VERSION_H
#define VERSION_H

#define FIRMWARE_VERSION "1.2.3"

#endif // VERSION_H
```

### 2. Build Validation (Always)

The workflow builds **all board variants** to ensure:

- ✅ **Compilation succeeds** - No syntax errors or missing dependencies
- ✅ **Firmware size** - Binary fits within partition limits (<1.5MB)
- ✅ **Build artifacts** - Generates downloadable firmware files

### 3. Automated PR Comments

The workflow posts two types of comments on your PR:

**Version Check Comment:**
- Tells you if version was incremented correctly
- Reminds you to update CHANGELOG.md
- Provides guidance on semantic versioning
- Shows instructions for creating release tag after merge

**Build Summary Comment:**
- Table showing firmware size for each board
- Percentage of partition used
- Success/failure status for each build
- Link to download firmware artifacts

## Workflow Triggers

The workflow runs on:

- **Pull requests to `main` branch**
  - Skips if only docs/markdown files changed
  - Skips if only flasher files changed
  
- **Manual trigger** (workflow_dispatch)
  - Can select specific board to build
  - Useful for testing

## Example PR Flow

### Scenario 1: Bug Fix PR

1. Create branch: `git checkout -b fix-wifi-bug`
2. Fix the bug in code
3. **Update version**: Change `1.2.3` → `1.2.4` in `version.h` (PATCH)
4. **Update CHANGELOG.md**:
   ```markdown
   ## [1.2.4] - 2025-11-09
   ### Fixed
   - Fixed WiFi reconnection issue after sleep
   ```
5. Commit and push
6. Create PR
7. Workflow validates:
   - ✅ Version incremented (1.2.3 → 1.2.4)
   - ✅ CHANGELOG has [1.2.4] entry
   - ✅ Both boards build successfully
8. Merge PR
9. Create tag: `git tag v1.2.4 && git push origin v1.2.4`

### Scenario 2: New Feature PR

1. Create branch: `git checkout -b add-feature`
2. Implement new feature
3. **Update version**: Change `1.2.3` → `1.3.0` in `version.h` (MINOR)
4. **Update CHANGELOG.md**:
   ```markdown
   ## [1.3.0] - 2025-11-09
   ### Added
   - New feature description
   ```
5. Commit and push
6. Create PR
7. Workflow validates and comments
8. Merge and tag

### Scenario 3: Breaking Change PR

1. Create branch: `git checkout -b breaking-change`
2. Implement breaking change
3. **Update version**: Change `1.2.3` → `2.0.0` in `version.h` (MAJOR)
4. **Update CHANGELOG.md**:
   ```markdown
   ## [2.0.0] - 2025-11-09
   ### Changed
   - BREAKING: Describe what changed and how to migrate
   ```
5. Workflow validates MAJOR version bump

### Scenario 4: Documentation-Only PR (No Version Change)

1. Create branch: `git checkout -b update-docs`
2. Update README.md or documentation
3. **Don't change version** (docs don't need version bump)
4. Create PR
5. Workflow comments:
   - ⚠️ Version not incremented (warning, not blocker)
   - ✅ Builds succeed
6. This is **OK** - docs-only changes don't need version increments
7. Merge PR without creating a tag

## Version Tracking is Optional

**If you don't have `common/src/version.h`:**
- Version check is skipped
- Workflow only validates builds
- You can still create releases manually with git tags

**To enable version tracking:**
1. Create `common/src/version.h` with:
   ```cpp
   #ifndef VERSION_H
   #define VERSION_H
   
   #define FIRMWARE_VERSION "1.0.0"
   
   #endif // VERSION_H
   ```
2. Create `CHANGELOG.md` following [Keep a Changelog](https://keepachangelog.com/)
3. Include version in your main sketch:
   ```cpp
   #include "version.h"
   void setup() {
     Serial.println("Firmware version: " FIRMWARE_VERSION);
   }
   ```

## Semantic Versioning Guide

**Format:** `MAJOR.MINOR.PATCH` (e.g., `1.2.3`)

- **MAJOR** (1.0.0 → 2.0.0): Breaking changes
  - Config format changes
  - API incompatibilities
  - Requires user action to upgrade
  
- **MINOR** (1.0.0 → 1.1.0): New features
  - Backward compatible additions
  - New config options (with defaults)
  - New functionality
  
- **PATCH** (1.0.0 → 1.0.1): Bug fixes
  - Backward compatible fixes
  - Performance improvements
  - Security patches

## Build Size Limits

The workflow enforces firmware size limits based on the partition scheme:

| Partition Scheme | Max Size | Usage Limit |
|------------------|----------|-------------|
| `min_spiffs` | 1.5 MB | <100% (builds fail if exceeded) |

**Tips to reduce firmware size:**
- Remove unused libraries
- Disable debug logging in production
- Use compiler optimization flags
- Remove unused features/code

## Manual Workflow Trigger

You can manually trigger the workflow from the Actions tab:

1. Go to Actions → Build and Validate PR
2. Click "Run workflow"
3. Select board: `all`, `esp32_dev`, or `esp32s3_dev`
4. Click "Run workflow"

Useful for:
- Testing build changes without creating a PR
- Building a specific board variant
- Debugging build issues

## Workflow Configuration

**Customize in `.github/workflows/build.yml`:**

```yaml
# Add more boards to the matrix
prepare:
  steps:
    - name: Set build matrix
      run: |
        echo 'matrix=["esp32_dev","esp32s3_dev","new_board"]' >> $GITHUB_OUTPUT

# Change paths that trigger the workflow
on:
  pull_request:
    paths-ignore:
      - '**.md'           # Ignore markdown files
      - 'docs/**'         # Ignore documentation
      - 'flasher/**'      # Ignore flasher files
```

## Troubleshooting

### "Version unchanged" warning on feature PR

**Fix:** Increment version in `common/src/version.h`

### "CHANGELOG missing entry" warning

**Fix:** Add section to CHANGELOG.md:
```markdown
## [1.2.3] - 2025-11-09
### Added/Changed/Fixed
- Your changes here
```

### Build fails: "Firmware too large"

**Fix:** 
- Remove unused code/libraries
- Check firmware size locally: `.\build.ps1 esp32_dev`
- Optimize code or increase partition size

### Workflow doesn't run

**Check:**
- PR is targeting `main` branch
- Changes aren't only in ignored paths (docs, markdown)
- Workflow permissions are correct (Settings → Actions)

### PR comment not posted

**Check:**
- Workflow permissions: "Read and write permissions"
- Check workflow logs in Actions tab
- Ensure PR is open (not draft)

## Best Practices

1. **Always update version for code changes**
   - Even small fixes deserve a PATCH bump
   - Helps with debugging and support

2. **Keep CHANGELOG.md up to date**
   - Write clear, user-facing descriptions
   - Group by Added/Changed/Fixed/Removed
   - Include migration notes for breaking changes

3. **Test locally before PR**
   - Run `.\build.ps1 all` to catch build errors early
   - Verify firmware size is reasonable

4. **Use meaningful commit messages**
   - Helps generate CHANGELOG entries
   - Makes git history useful

5. **Tag immediately after merge**
   - Don't forget to create the release tag
   - Triggers automatic deployment

## Related Documentation

- [FIRST_DEPLOYMENT.md](FIRST_DEPLOYMENT.md) - First-time deployment checklist
- [docs/WEB_FLASHER_SETUP.md](docs/WEB_FLASHER_SETUP.md) - Web flasher configuration
- [DEPLOYMENT_SUMMARY.md](DEPLOYMENT_SUMMARY.md) - Complete deployment overview
