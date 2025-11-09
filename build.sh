#!/bin/bash
# ESP32 Multi-Board Template - Build Script (Linux/macOS)
# Builds firmware for ESP32 boards using Arduino CLI

set -e  # Exit on error

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
ARDUINO_CLI="${ARDUINO_CLI:-arduino-cli}"
ESP32_CORE_VERSION="3.3.2"

# Board configuration (FQBN = Fully Qualified Board Name)
declare -A BOARDS
BOARDS[esp32_dev]="esp32:esp32:esp32:PartitionScheme=min_spiffs"
BOARDS[esp32s3_dev]="esp32:esp32:esp32s3:PartitionScheme=min_spiffs"

# Function to print colored messages
print_info() {
    echo -e "${BLUE}$1${NC}"
}

print_success() {
    echo -e "${GREEN}$1${NC}"
}

print_error() {
    echo -e "${RED}$1${NC}"
}

print_warning() {
    echo -e "${YELLOW}$1${NC}"
}

# Function to check if Arduino CLI is installed
check_arduino_cli() {
    if ! command -v "$ARDUINO_CLI" &> /dev/null; then
        print_error "Arduino CLI not found!"
        echo "Please install Arduino CLI from: https://arduino.github.io/arduino-cli/latest/installation/"
        echo "Or set ARDUINO_CLI environment variable to the path of arduino-cli"
        exit 1
    fi
    
    print_info "Arduino CLI found: $($ARDUINO_CLI version)"
}

# Function to check ESP32 core installation
check_esp32_core() {
    if ! $ARDUINO_CLI core list | grep -q "esp32:esp32.*$ESP32_CORE_VERSION"; then
        print_warning "ESP32 core $ESP32_CORE_VERSION not installed"
        print_info "Installing ESP32 core..."
        $ARDUINO_CLI core update-index
        $ARDUINO_CLI core install "esp32:esp32@$ESP32_CORE_VERSION"
    else
        print_info "ESP32 core $ESP32_CORE_VERSION already installed"
    fi
}

# Function to check PubSubClient library
check_libraries() {
    if ! $ARDUINO_CLI lib list | grep -q "PubSubClient"; then
        print_warning "PubSubClient library not installed"
        print_info "Installing PubSubClient..."
        $ARDUINO_CLI lib install "PubSubClient"
    else
        print_info "PubSubClient library already installed"
    fi
}

# Function to build a board
build_board() {
    local board_name=$1
    local fqbn=${BOARDS[$board_name]}
    
    if [ -z "$fqbn" ]; then
        print_error "Unknown board: $board_name"
        echo "Available boards: ${!BOARDS[@]}"
        exit 1
    fi
    
    local board_dir="boards/$board_name"
    local sketch_file="$board_dir/$board_name.ino"
    local build_dir="build/$board_name"
    
    print_info "═══════════════════════════════════════"
    print_info "Building: $board_name"
    print_info "═══════════════════════════════════════"
    
    # Clean build directory
    rm -rf "$build_dir"
    mkdir -p "$build_dir"
    
    # Copy all .cpp, .h, and .ino.inc files from common/src/ to board directory
    print_info "Copying common source files..."
    find common/src \( -name "*.cpp" -o -name "*.h" -o -name "*.ino.inc" \) | while read -r file; do
        filename=$(basename "$file")
        cp "$file" "$board_dir/"
        echo "Copied $filename"
    done
    
    # Compile
    print_info "Compiling..."
    $ARDUINO_CLI compile \
        --fqbn "$fqbn" \
        --output-dir "$build_dir" \
        --build-property "compiler.cpp.extra_flags=-include board_config.h" \
        "$sketch_file"
    
    # Clean up copied files
    print_info "Cleaning up copied files..."
    find common/src \( -name "*.cpp" -o -name "*.h" -o -name "*.ino.inc" \) | while read -r file; do
        filename=$(basename "$file")
        rm -f "$board_dir/$filename"
    done
    
    # Check if build succeeded
    if [ -f "$build_dir/$board_name.ino.bin" ]; then
        local size=$(du -h "$build_dir/$board_name.ino.bin" | cut -f1)
        print_success "Build successful"
        print_success "Firmware size: $size"
        echo ""
        ls -lh "$build_dir"/*.bin
    else
        print_error "Build failed"
        exit 1
    fi
}

# Main script
echo "ESP32 Multi-Board Template - Build Script"
echo ""

# Check prerequisites
check_arduino_cli
check_esp32_core
check_libraries

# Parse command line arguments
if [ $# -eq 0 ]; then
    print_error "Usage: $0 <board_name> | all"
    echo "Available boards: ${!BOARDS[@]}"
    exit 1
fi

if [ "$1" = "all" ]; then
    # Build all boards
    for board in "${!BOARDS[@]}"; do
        build_board "$board"
        echo ""
    done
    print_success "═══════════════════════════════════════"
    print_success "All boards built successfully!"
    print_success "═══════════════════════════════════════"
else
    # Build specific board
    build_board "$1"
fi
