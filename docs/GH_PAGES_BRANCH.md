# gh-pages Branch Guidelines

## Purpose
The `gh-pages` branch is **auto-generated** by GitHub Actions and should NOT be manually edited.

## What's in gh-pages
- `flasher/` directory (HTML, JS, CSS, manifests)
- `flasher/firmware/` directory with all release binaries
- `flasher/latest.json` metadata file

## What's NOT in main branch
- Firmware binaries (*.bin files)
- `flasher/firmware/` directory
- `flasher/latest.json` file

## How It Works
1. Developer creates git tag: `v1.0.0`
2. GitHub Actions workflow (`release.yml`) triggers
3. Workflow builds all boards, generates manifests
4. Workflow commits to `gh-pages` branch
5. GitHub Pages auto-deploys from `gh-pages`

## Manual gh-pages Operations (Rarely Needed)

**View gh-pages branch:**
```bash
git fetch origin gh-pages
git checkout gh-pages
ls -la flasher/
```

**Force rebuild gh-pages (emergency):**
```bash
# Delete remote gh-pages branch
git push origin --delete gh-pages

# Re-tag to trigger rebuild
git tag -d v1.0.0
git push origin :refs/tags/v1.0.0
git tag v1.0.0
git push origin v1.0.0
```

## .gitignore for gh-pages

If you need to create a `.gitignore` for the `gh-pages` branch, it should ALLOW firmware files:

```gitignore
# gh-pages branch .gitignore
# (opposite of main branch - we WANT binaries here)

# Only ignore OS files
.DS_Store
Thumbs.db
Desktop.ini
```

**Note:** GitHub Actions workflow handles this automatically. You typically don't need a custom `.gitignore` on `gh-pages`.

## Troubleshooting

**gh-pages not updating:**
1. Check GitHub Actions logs (Actions tab)
2. Verify workflow completed successfully
3. Check GitHub Pages settings enabled
4. Wait 1-2 minutes for Pages rebuild
5. Clear browser cache

**gh-pages has conflicts:**
- This shouldn't happen (workflow force-pushes)
- If it does, delete and recreate: see "Force rebuild" above

**Firmware files in main branch by accident:**
1. Remove files: `git rm flasher/firmware/*.bin`
2. Update `.gitignore` to prevent re-adding
3. Commit and push
4. Files still exist in `gh-pages` (correct!)
