#!/bin/bash
# Arduino CLI Upload Script for ESP32 Multi-Board Template
# This script uploads the Arduino sketch using Arduino CLI
# Dynamically discovers boards from boards/*/board.json files

set -e

# Default values
BOARD=""
PORT=""
ERASE=false

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Function to discover boards from board.json files
discover_boards() {
    declare -gA BOARD_NAMES
    declare -gA BOARD_FQBNS
    declare -gA BOARD_PATHS
    
    for board_dir in boards/*/; do
        if [ -d "$board_dir" ]; then
            board_id=$(basename "$board_dir")
            board_json="$board_dir/board.json"
            
            if [ -f "$board_json" ]; then
                # Parse JSON using grep and sed (portable approach)
                name=$(grep '"name"' "$board_json" | sed 's/.*"name"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/')
                fqbn=$(grep '"fqbn"' "$board_json" | sed 's/.*"fqbn"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/')
                
                if [ -n "$name" ] && [ -n "$fqbn" ]; then
                    BOARD_NAMES[$board_id]="$name"
                    BOARD_FQBNS[$board_id]="$fqbn"
                    BOARD_PATHS[$board_id]="boards/$board_id"
                fi
            fi
        fi
    done
}

# Function to list available boards
list_boards() {
    echo -e "${CYAN}Available boards:${NC}"
    for board_id in "${!BOARD_NAMES[@]}"; do
        echo -e "  ${GREEN}- $board_id (${BOARD_NAMES[$board_id]})${NC}"
    done | sort
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -b|--board)
            BOARD="$2"
            shift 2
            ;;
        -p|--port)
            PORT="$2"
            shift 2
            ;;
        -e|--erase)
            ERASE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -b, --board <name>   Board to upload to"
            echo "  -p, --port <port>    Serial port (e.g., /dev/ttyUSB0)"
            echo "  -e, --erase          Erase flash before upload"
            echo "  -h, --help           Show this help message"
            echo ""
            echo "Example: $0 -b esp32_dev -p /dev/ttyUSB0"
            exit 0
            ;;
        *)
            echo -e "${RED}ERROR: Unknown option: $1${NC}"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

# Discover available boards
discover_boards

if [ ${#BOARD_NAMES[@]} -eq 0 ]; then
    echo -e "${RED}ERROR: No boards found in boards/ directory!${NC}"
    echo -e "${YELLOW}Ensure each board has a board.json file.${NC}"
    exit 1
fi

# If no board specified, show available boards
if [ -z "$BOARD" ]; then
    list_boards
    echo ""
    echo -e "${YELLOW}Usage: $0 -b <board_name> [-p <port>] [-e]${NC}"
    echo -e "${CYAN}Example: $0 -b esp32_dev -p /dev/ttyUSB0${NC}"
    exit 0
fi

# Validate board selection
if [ -z "${BOARD_NAMES[$BOARD]}" ]; then
    echo -e "${RED}ERROR: Board '$BOARD' not found!${NC}"
    echo ""
    list_boards
    exit 1
fi

BOARD_NAME="${BOARD_NAMES[$BOARD]}"
BOARD_FQBN="${BOARD_FQBNS[$BOARD]}"
SKETCH_PATH="${BOARD_PATHS[$BOARD]}"
BUILD_DIR="build/$BOARD"

echo -e "${CYAN}========================================${NC}"
echo -e "${GREEN}Uploading to: $BOARD_NAME${NC}"
echo -e "${CYAN}========================================${NC}"

# Check if Arduino CLI is installed
if ! command -v arduino-cli &> /dev/null; then
    echo -e "${RED}ERROR: Arduino CLI not found!${NC}"
    echo ""
    echo -e "${YELLOW}Please install Arduino CLI:${NC}"
    echo -e "${CYAN}  https://arduino.github.io/arduino-cli/latest/installation/${NC}"
    exit 1
fi

# Auto-detect port if not specified
if [ -z "$PORT" ]; then
    echo ""
    echo -e "${YELLOW}Auto-detecting board...${NC}"
    arduino-cli board list
    
    # Try to find ESP32 board on common ports
    for test_port in /dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyACM0 /dev/ttyACM1 /dev/cu.usbserial* /dev/cu.SLAB_USBtoUART*; do
        if [ -e "$test_port" ]; then
            PORT="$test_port"
            echo -e "${GREEN}Detected board on port: $PORT${NC}"
            break
        fi
    done
    
    if [ -z "$PORT" ]; then
        echo -e "${RED}ERROR: No board detected!${NC}"
        echo ""
        echo -e "${YELLOW}Please ensure:${NC}"
        echo -e "${YELLOW}  1. The ESP32 board is connected via USB${NC}"
        echo -e "${YELLOW}  2. The USB drivers are installed (CP210x or CH340)${NC}"
        echo -e "${YELLOW}  3. No other program is using the serial port${NC}"
        echo ""
        echo -e "${CYAN}Or specify port manually: $0 -b $BOARD -p /dev/ttyUSB0${NC}"
        exit 1
    fi
fi

echo ""
echo -e "${CYAN}Port: $PORT${NC}"
echo -e "${CYAN}Board: $BOARD_NAME${NC}"
echo -e "${CYAN}FQBN: $BOARD_FQBN${NC}"

# Handle erase flash if requested
if [ "$ERASE" = true ]; then
    echo ""
    echo -e "${YELLOW}ERASING FLASH MEMORY...${NC}"
    echo -e "${YELLOW}This will delete all stored configuration and firmware.${NC}"
    arduino-cli burn-bootloader --fqbn "$BOARD_FQBN" --port "$PORT" --programmer esptool
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Flash erased successfully!${NC}"
        echo -e "${CYAN}Now uploading fresh firmware...${NC}"
    else
        echo ""
        echo -e "${RED}Flash erase failed!${NC}"
        exit 1
    fi
fi

# Check if build exists
BINARY_FILE="$BUILD_DIR/$BOARD.ino.bin"
if [ -f "$BINARY_FILE" ]; then
    echo -e "${GREEN}Found existing build${NC}"
    echo ""
    echo -e "${YELLOW}Uploading to $PORT...${NC}"
    arduino-cli upload --fqbn "$BOARD_FQBN" --port "$PORT" --input-dir "$BUILD_DIR" "$SKETCH_PATH"
    
    if [ $? -ne 0 ]; then
        echo ""
        echo -e "${RED}Upload failed!${NC}"
        echo ""
        echo -e "${YELLOW}Troubleshooting tips:${NC}"
        echo -e "${YELLOW}  1. Check that the board is connected and powered${NC}"
        echo -e "${YELLOW}  2. Try a different USB cable or port${NC}"
        echo -e "${YELLOW}  3. Close any serial monitors or other programs using the port${NC}"
        echo -e "${YELLOW}  4. Press the BOOT button on the ESP32 during upload (if needed)${NC}"
        echo -e "${YELLOW}  5. Verify correct board: $0 -b $BOARD${NC}"
        echo -e "${YELLOW}  6. Try erasing flash first: $0 -b $BOARD -p $PORT -e${NC}"
        exit 1
    fi
else
    echo -e "${RED}No existing build found at: $BINARY_FILE${NC}"
    echo ""
    echo -e "${YELLOW}Please build the firmware first:${NC}"
    echo -e "${CYAN}  ./build.sh $BOARD${NC}"
    echo ""
    echo -e "${YELLOW}Or use compile + upload in one step:${NC}"
    echo -e "${CYAN}  arduino-cli compile --fqbn $BOARD_FQBN --upload --port $PORT $SKETCH_PATH${NC}"
    exit 1
fi

echo ""
echo -e "${CYAN}========================================${NC}"
echo -e "${GREEN}Upload successful!${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""
echo -e "${CYAN}Next steps:${NC}"
echo -e "${YELLOW}  1. Open serial monitor: arduino-cli monitor -p $PORT -c baudrate=115200${NC}"
echo -e "${YELLOW}  2. Connect to device AP: esp32-XXXXXX${NC}"
echo -e "${YELLOW}  3. Configure at: http://192.168.4.1${NC}"
