#!/bin/bash
# ESP32 Multi-Board Template - Build Script (Linux/macOS)
# Builds firmware for ESP32 boards using Arduino CLI

set -e  # Exit on error

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
GRAY='\033[0;90m'
NC='\033[0m' # No Color

# Configuration
ARDUINO_CLI="${ARDUINO_CLI:-arduino-cli}"
ESP32_CORE_VERSION="3.3.2"

# Board configuration will be loaded dynamically from board.json files
declare -A BOARDS
declare -A BOARD_MANAGER_URLS

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

print_gray() {
    echo -e "${GRAY}$1${NC}"
}

# Function to load board configurations dynamically
load_board_configurations() {
    local boards_dir="boards"
    
    print_info "Loading board configurations from $boards_dir/..."
    
    if [ ! -d "$boards_dir" ]; then
        print_error "ERROR: boards/ directory not found"
        exit 1
    fi
    
    local board_count=0
    
    for board_dir in "$boards_dir"/*; do
        print_gray "Checking directory: $board_dir"
        
        if [ ! -d "$board_dir" ]; then
            print_gray "  Skipping (not a directory)"
            continue
        fi
        
        local board_name=$(basename "$board_dir")
        local board_json="$board_dir/board.json"
        
        if [ ! -f "$board_json" ]; then
            print_warning "WARNING: Skipping $board_name - no board.json found"
            continue
        fi
        
        # Parse JSON (using Python for portability)
        if ! command -v python3 &> /dev/null && ! command -v python &> /dev/null; then
            print_error "ERROR: Python is required to parse board.json files"
            exit 1
        fi
        
        local python_cmd=$(command -v python3 || command -v python)
        
        print_gray "Parsing $board_json..."
        
        # Parse JSON with explicit error handling (set -e compatible)
        set +e  # Temporarily disable exit on error
        board_display_name=$($python_cmd -c "import json; print(json.load(open('$board_json')).get('name', ''))" 2>/dev/null)
        board_fqbn=$($python_cmd -c "import json; print(json.load(open('$board_json')).get('fqbn', ''))" 2>/dev/null)
        board_manager_url=$($python_cmd -c "import json; print(json.load(open('$board_json')).get('board_manager_url', ''))" 2>/dev/null)
        set -e  # Re-enable exit on error
        
        if [ -z "$board_display_name" ] || [ -z "$board_fqbn" ]; then
            print_warning "WARNING: Skipping $board_name - board.json missing 'name' or 'fqbn'"
            print_gray "  name='$board_display_name', fqbn='$board_fqbn'"
            continue
        fi
        
        print_gray "Storing board configuration..."
        BOARDS[$board_name]=$board_fqbn
        print_gray "  Stored FQBN"
        BOARD_MANAGER_URLS[$board_name]=$board_manager_url
        print_gray "  Stored board manager URL"
        
        print_gray "Discovered board: $board_name ($board_display_name)"
        print_gray "  FQBN: $board_fqbn"
        if [ -n "$board_manager_url" ]; then
            print_gray "  Board Manager URL: $board_manager_url"
        fi
        
        print_gray "Incrementing counter..."
        board_count=$((board_count + 1))
        print_gray "  Board count now: $board_count"
        print_gray "Continuing to next directory..."
    done
    
    print_info "Finished scanning directories"
    
    if [ $board_count -eq 0 ]; then
        print_error "ERROR: No valid boards found. Each board needs a board.json file."
        exit 1
    fi
    
    print_success "Loaded $board_count board(s) successfully"
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

# Function to discover and configure board manager URLs
discover_board_manager_urls() {
    local board_arg=$1
    
    print_info "═══════════════════════════════════════"
    print_info "Discovering Board Manager URLs..."
    print_info "═══════════════════════════════════════"
    
    # Collect unique board manager URLs
    declare -A unique_urls
    
    if [ "$board_arg" = "all" ]; then
        # Collect from all boards
        print_gray "Scanning all boards for board manager URLs..."
        for board_name in "${!BOARDS[@]}"; do
            local url="${BOARD_MANAGER_URLS[$board_name]}"
            if [ -n "$url" ]; then
                print_gray "  Found URL for $board_name: $url"
                unique_urls[$url]=1
            else
                print_gray "  No URL for $board_name (will use default)"
            fi
        done
    else
        # Collect from specific board
        print_gray "Checking board manager URL for: $board_arg"
        local url="${BOARD_MANAGER_URLS[$board_arg]}"
        if [ -n "$url" ]; then
            print_gray "  Found URL: $url"
            unique_urls[$url]=1
        else
            print_gray "  No URL configured (will use default)"
        fi
    fi
    
    # Add discovered URLs to arduino-cli config
    if [ ${#unique_urls[@]} -eq 0 ]; then
        print_info "No additional board manager URLs to add"
    else
        print_info "Adding ${#unique_urls[@]} board manager URL(s) to arduino-cli config..."
        for url in "${!unique_urls[@]}"; do
            print_gray "  Adding: $url"
            $ARDUINO_CLI config add board_manager.additional_urls "$url" 2>/dev/null || true
        done
    fi
    
    # Update core index after adding URLs
    print_info "Updating core index..."
    $ARDUINO_CLI core update-index
    print_success "Core index updated successfully"
}

# Function to install required cores
install_cores() {
    local board_arg=$1
    
    print_info ""
    print_info "Checking Core Installations..."
    
    # Collect unique cores to install (extract package:arch from FQBN)
    declare -A unique_cores
    
    if [ "$board_arg" = "all" ]; then
        print_gray "Extracting cores from all board FQBNs..."
        for board_name in "${!BOARDS[@]}"; do
            local fqbn="${BOARDS[$board_name]}"
            # Extract package:arch from FQBN (format: package:arch:board)
            if [[ $fqbn =~ ^([^:]+:[^:]+) ]]; then
                local core="${BASH_REMATCH[1]}"
                print_gray "  $board_name requires core: $core"
                unique_cores[$core]=1
            else
                print_warning "  WARNING: Could not parse FQBN for $board_name: $fqbn"
            fi
        done
    else
        print_gray "Extracting core from board FQBN..."
        local fqbn="${BOARDS[$board_arg]}"
        if [[ $fqbn =~ ^([^:]+:[^:]+) ]]; then
            local core="${BASH_REMATCH[1]}"
            print_gray "  Requires core: $core"
            unique_cores[$core]=1
        else
            print_error "ERROR: Could not parse FQBN: $fqbn"
            exit 1
        fi
    fi
    
    print_info "Need to install ${#unique_cores[@]} unique core(s)"
    
    # Install each unique core
    for core in "${!unique_cores[@]}"; do
        print_info "Checking core: $core"
        if ! $ARDUINO_CLI core list | grep -q "^$core"; then
            print_warning "Core $core not installed"
            print_info "Installing $core..."
            
            # For esp32:esp32, pin to specific version for consistency
            if [ "$core" = "esp32:esp32" ]; then
                print_gray "  Using pinned version: $ESP32_CORE_VERSION"
                $ARDUINO_CLI core install "$core@$ESP32_CORE_VERSION"
            else
                # For other cores, install latest version
                print_gray "  Using latest version"
                $ARDUINO_CLI core install "$core"
            fi
            print_success "Core $core installed successfully"
        else
            print_success "Core $core already installed"
        fi
    done
}

# Function to check PubSubClient library
check_libraries() {
    print_info ""
    print_info "Checking Library Installation..."
    
    # Read required libraries from libraries.txt
    if [ ! -f "libraries.txt" ]; then
        print_warning "No libraries.txt file found, skipping library installation"
        return
    fi
    
    while IFS= read -r lib || [ -n "$lib" ]; do
        # Skip comments and empty lines
        [[ "$lib" =~ ^#.*$ ]] && continue
        [[ -z "$lib" ]] && continue
        
        # Trim whitespace
        lib=$(echo "$lib" | xargs)
        
        print_info "Checking library: $lib"
        if ! $ARDUINO_CLI lib list | grep -q "$lib"; then
            print_warning "$lib library not installed"
            print_info "Installing $lib..."
            $ARDUINO_CLI lib install "$lib"
            print_success "$lib installed successfully"
        else
            print_success "$lib library already installed"
        fi
    done < libraries.txt
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
    
    # Update the copied package_config.h with values from package.json
    local copied_package_config="$board_dir/package_config.h"
    if [ -f "$copied_package_config" ] && [ -f "package.json" ]; then
        local python_cmd=$(command -v python3 || command -v python)
        if [ -n "$python_cmd" ]; then
            local pkg_name=$($python_cmd -c "import json; print(json.load(open('package.json')).get('name', ''))" 2>/dev/null || echo "")
            local pkg_display_name=$($python_cmd -c "import json; print(json.load(open('package.json')).get('displayName', ''))" 2>/dev/null || echo "")
            local pkg_display_name_short=$($python_cmd -c "import json; print(json.load(open('package.json')).get('displayNameShort', ''))" 2>/dev/null || echo "")
            
            if [ -n "$pkg_name" ]; then
                sed -i.tmp "s|#define PACKAGE_NAME \".*\"|#define PACKAGE_NAME \"$pkg_name\"|g" "$copied_package_config"
            fi
            if [ -n "$pkg_display_name" ]; then
                sed -i.tmp "s|#define PACKAGE_DISPLAY_NAME \".*\"|#define PACKAGE_DISPLAY_NAME \"$pkg_display_name\"|g" "$copied_package_config"
            fi
            if [ -n "$pkg_display_name_short" ]; then
                sed -i.tmp "s|#define PACKAGE_DISPLAY_NAME_SHORT \".*\"|#define PACKAGE_DISPLAY_NAME_SHORT \"$pkg_display_name_short\"|g" "$copied_package_config"
            fi
            rm -f "$copied_package_config.tmp"
            print_gray "Updated copied package_config.h with values from package.json"
        fi
    fi
    
    # Compile
    print_info "Compiling..."
    $ARDUINO_CLI compile \
        --fqbn "$fqbn" \
        --output-dir "$build_dir" \
        --build-property "compiler.cpp.extra_flags=-include board_config.h" \
        "$sketch_file"
    
    local build_result=$?
    
    # Clean up copied files
    print_info "Cleaning up copied files..."
    find common/src \( -name "*.cpp" -o -name "*.h" -o -name "*.ino.inc" \) | while read -r file; do
        filename=$(basename "$file")
        rm -f "$board_dir/$filename"
    done
    
    # Check if build succeeded
    if [ $build_result -eq 0 ] && [ -f "$build_dir/$board_name.ino.bin" ]; then
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

# Parse command line arguments first
if [ $# -eq 0 ]; then
    print_error "Usage: $0 <board_name> | all"
    echo "Available boards: Run with a board name to see discovered boards"
    exit 1
fi

# Load board configurations
load_board_configurations

# Validate board argument if not "all"
if [ "$1" != "all" ]; then
    if [ -z "${BOARDS[$1]}" ]; then
        print_error "ERROR: Unknown board '$1'"
        print_warning "Available boards: ${!BOARDS[@]}"
        exit 1
    fi
fi

# Check prerequisites
check_arduino_cli

# Discover board manager URLs based on boards to build
if [ "$1" = "all" ]; then
    discover_board_manager_urls "all"
    install_cores "all"
else
    discover_board_manager_urls "$1"
    install_cores "$1"
fi

check_libraries

# Build boards
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
