#!/usr/bin/env bash
# Generate ESP Web Tools manifest JSON files for each board
# Usage: generate_manifests.sh <tag> <artifacts_dir> <output_dir> <github_user> <github_repo>
set -euo pipefail

TAG=${1:-}
ARTIFACTS_DIR=${2:-artifacts}
OUTPUT_DIR=${3:-flasher}
GITHUB_USER=${4:-YOUR-USERNAME}
GITHUB_REPO=${5:-esp32-multiboard-template}

if [ -z "$TAG" ]; then
  echo "Usage: $0 <tag> [artifacts_dir] [output_dir] [github_user] [github_repo]"
  echo "Example: $0 v1.0.0 artifacts flasher myusername esp32-multiboard-template"
  exit 2
fi

# Remove 'v' prefix from tag if present
VERSION="${TAG#v}"

# Ensure jq exists
if ! command -v jq >/dev/null 2>&1; then
  echo "jq is required but not installed. Please install jq."
  exit 1
fi

# Board configuration
declare -A NAMES
declare -A CHIP_FAMILIES
NAMES[esp32_dev]="ESP32 DevKit V1"
NAMES[esp32s3_dev]="ESP32-S3 DevKit"
CHIP_FAMILIES[esp32_dev]="ESP32"
CHIP_FAMILIES[esp32s3_dev]="ESP32-S3"

# ESP32 memory offsets (in decimal)
BOOTLOADER_OFFSET_ESP32=4096      # 0x1000
BOOTLOADER_OFFSET_ESP32S3=0       # 0x0000
PARTITIONS_OFFSET=32768           # 0x8000
FIRMWARE_OFFSET=65536             # 0x10000

# Create output directory
mkdir -p "$OUTPUT_DIR"

echo "Generating ESP Web Tools manifests for version ${VERSION}..."

# Find all main firmware binaries and generate manifests
for firmware_bin in "$ARTIFACTS_DIR"/*-v*.bin; do
  [ -f "$firmware_bin" ] || continue
  
  # Skip bootloader and partition files
  if [[ "$firmware_bin" == *".bootloader.bin" ]] || [[ "$firmware_bin" == *".partitions.bin" ]]; then
    continue
  fi
  
  filename=$(basename "$firmware_bin")
  # Extract board name (e.g., "esp32_dev" from "esp32_dev-v1.0.0.bin")
  board=$(echo "$filename" | sed -E 's/-v.*//')
  
  # Construct filenames for all parts
  bootloader_file="${board}-v${VERSION}.bootloader.bin"
  partitions_file="${board}-v${VERSION}.partitions.bin"
  firmware_file="${board}-v${VERSION}.bin"
  
  # Check if all required files exist
  if [ ! -f "$ARTIFACTS_DIR/$bootloader_file" ]; then
    echo "⚠️  Warning: Bootloader not found for $board: $bootloader_file"
    continue
  fi
  
  if [ ! -f "$ARTIFACTS_DIR/$partitions_file" ]; then
    echo "⚠️  Warning: Partitions file not found for $board: $partitions_file"
    continue
  fi
  
  # Generate URLs pointing to GitHub Pages (to avoid CORS issues)
  base_url="https://${GITHUB_USER}.github.io/${GITHUB_REPO}/firmware/${TAG}"
  bootloader_url="${base_url}/${bootloader_file}"
  partitions_url="${base_url}/${partitions_file}"
  firmware_url="${base_url}/${firmware_file}"
  
  # Get board-specific values
  display_name="${NAMES[$board]:-$board}"
  chip_family="${CHIP_FAMILIES[$board]:-ESP32}"
  
  # ESP32-S3 has bootloader at offset 0, ESP32 at 0x1000
  if [ "$chip_family" == "ESP32-S3" ]; then
    bootloader_offset=$BOOTLOADER_OFFSET_ESP32S3
  else
    bootloader_offset=$BOOTLOADER_OFFSET_ESP32
  fi
  
  manifest_file="$OUTPUT_DIR/manifest_${board}.json"
  
  # Create manifest JSON
  jq -n \
    --arg name "ESP32 Template for ${display_name}" \
    --arg version "$VERSION" \
    --arg chip_family "$chip_family" \
    --arg bootloader_url "$bootloader_url" \
    --argjson bootloader_offset "$bootloader_offset" \
    --arg partitions_url "$partitions_url" \
    --argjson partitions_offset "$PARTITIONS_OFFSET" \
    --arg firmware_url "$firmware_url" \
    --argjson firmware_offset "$FIRMWARE_OFFSET" \
    '{
      name: $name,
      version: $version,
      new_install_prompt_erase: true,
      builds: [{
        chipFamily: $chip_family,
        parts: [
          { path: $bootloader_url, offset: $bootloader_offset },
          { path: $partitions_url, offset: $partitions_offset },
          { path: $firmware_url, offset: $firmware_offset }
        ]
      }]
    }' > "$manifest_file"
  
  echo "✅ Generated manifest: $manifest_file"
done

echo ""
echo "Manifest generation complete!"
echo "Manifests saved to: $OUTPUT_DIR/"
