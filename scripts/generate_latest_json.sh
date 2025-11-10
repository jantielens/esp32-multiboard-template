#!/usr/bin/env bash
# Generate latest.json manifest for flasher
# Usage: generate_latest_json.sh <tag> <artifacts_dir> <output_path> <github_user> <github_repo>
set -euo pipefail

TAG=${1:-}
ARTIFACTS_DIR=${2:-artifacts}
OUT_FILE=${3:-latest.json}
# Note: These defaults are just fallbacks for manual usage.
# In CI/CD, these are automatically filled with actual values:
# - ${{ github.repository_owner }} -> your GitHub username
# - ${{ github.event.repository.name }} -> your repo name
GITHUB_USER=${4:-YOUR-USERNAME}
GITHUB_REPO=${5:-esp32-multiboard-template}

if [ -z "$TAG" ]; then
  echo "Usage: $0 <tag> [artifacts_dir] [output_path] [github_user] [github_repo]"
  exit 2
fi

PUBLISHED_AT=$(date -u +%Y-%m-%dT%H:%M:%SZ)

# Ensure jq exists
if ! command -v jq >/dev/null 2>&1; then
  echo "jq is required but not installed. Please install jq."
  exit 1
fi

# Dynamically load board configurations from board.json files
declare -A NAMES

echo "Loading board configurations..."
for board_dir in boards/*; do
  if [ ! -d "$board_dir" ]; then
    continue
  fi
  
  board_name=$(basename "$board_dir")
  board_json="$board_dir/board.json"
  
  if [ ! -f "$board_json" ]; then
    continue
  fi
  
  # Parse board.json
  display_name=$(jq -r '.name // empty' "$board_json")
  
  if [ -z "$display_name" ]; then
    continue
  fi
  
  NAMES[$board_name]="$display_name"
  echo "  Loaded: $board_name -> $display_name"
done

if [ ${#NAMES[@]} -eq 0 ]; then
  echo "ERROR: No boards found in boards/ directory"
  exit 1
fi

TMP=$(mktemp)
trap 'rm -f "$TMP"' EXIT

jq -n --arg tag "$TAG" --arg published_at "$PUBLISHED_AT" '{tag_name: $tag, published_at: $published_at, assets: []}' > "$TMP"

for f in "$ARTIFACTS_DIR"/*-v*.bin; do
  [ -f "$f" ] || continue
  filename=$(basename "$f")
  
  # Skip bootloader and partition files for the main assets list
  if [[ "$filename" == *".bootloader.bin" ]] || [[ "$filename" == *".partitions.bin" ]]; then
    continue
  fi
  
  # Sanitize inputs (remove CR and LF)
  filename=$(printf "%s" "$filename" | tr -d '\r\n')
  TAG=$(printf "%s" "$TAG" | tr -d '\r\n')
  board=$(echo "$filename" | sed -E 's/-v.*//')
  
  # Construct URLs for all files (use GitHub Pages to avoid CORS issues)
  base_url="https://${GITHUB_USER}.github.io/${GITHUB_REPO}/firmware/${TAG}"
  firmware_url="${base_url}/${filename}"
  bootloader_file="${board}-v${TAG#v}.bootloader.bin"
  bootloader_url="${base_url}/${bootloader_file}"
  partitions_file="${board}-v${TAG#v}.partitions.bin"
  partitions_url="${base_url}/${partitions_file}"
  
  # Remove any stray CR/LF that may exist in variables
  firmware_url=$(printf "%s" "$firmware_url" | tr -d '\r\n')
  bootloader_url=$(printf "%s" "$bootloader_url" | tr -d '\r\n')
  partitions_url=$(printf "%s" "$partitions_url" | tr -d '\r\n')
  
  display_name="${NAMES[$board]:-$board}"
  
  # Append asset with all binary URLs
  jq --arg board "$board" \
     --arg filename "$filename" \
     --arg url "$firmware_url" \
     --arg bootloader_url "$bootloader_url" \
     --arg partitions_url "$partitions_url" \
     --arg display_name "$display_name" \
     '.assets += [{
       board: $board,
       filename: $filename,
       url: $url,
       bootloader_url: $bootloader_url,
       partitions_url: $partitions_url,
       display_name: $display_name
     }]' "$TMP" > "$TMP.tmp" && mv "$TMP.tmp" "$TMP"
done

# Sort assets by board for determinism
jq '.assets |= sort_by(.board)' "$TMP" > "$TMP.tmp" && mv "$TMP.tmp" "$TMP"

mv "$TMP" "$OUT_FILE"

echo "Generated $OUT_FILE"
