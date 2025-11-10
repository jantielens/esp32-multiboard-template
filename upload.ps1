# Arduino CLI Upload Script for ESP32 Multi-Board Template
# This script uploads the Arduino sketch using Arduino CLI
# Dynamically discovers boards from boards/*/board.json files

param(
    [Parameter(Mandatory=$false)]
    [string]$Board = "",
    
    [Parameter(Mandatory=$false)]
    [string]$Port = "",
    
    [Parameter(Mandatory=$false)]
    [switch]$Erase
)

# Function to discover boards from board.json files
function Get-AvailableBoards {
    $boardsHash = @{}
    $boardDirs = Get-ChildItem -Path "boards" -Directory
    
    foreach ($dir in $boardDirs) {
        $boardJsonPath = Join-Path $dir.FullName "board.json"
        if (Test-Path $boardJsonPath) {
            try {
                $boardJson = Get-Content $boardJsonPath -Raw | ConvertFrom-Json
                $boardId = $dir.Name
                $boardsHash[$boardId] = @{
                    Name = $boardJson.name
                    FQBN = $boardJson.fqbn
                    Path = "boards/$boardId"
                }
            } catch {
                Write-Host "Warning: Failed to parse $boardJsonPath" -ForegroundColor Yellow
            }
        }
    }
    
    return $boardsHash
}

# Discover available boards
$boards = Get-AvailableBoards

if ($boards.Count -eq 0) {
    Write-Host "ERROR: No boards found in boards/ directory!" -ForegroundColor Red
    Write-Host "Ensure each board has a board.json file." -ForegroundColor Yellow
    exit 1
}

# If no board specified, show available boards
if (!$Board) {
    Write-Host "Available boards:" -ForegroundColor Cyan
    $boards.Keys | Sort-Object | ForEach-Object {
        Write-Host "  - $_ ($($boards[$_].Name))" -ForegroundColor Green
    }
    Write-Host "`nUsage: .\upload.ps1 -Board <board_name> [-Port <COM_port>] [-Erase]" -ForegroundColor Yellow
    Write-Host "Example: .\upload.ps1 -Board esp32_dev -Port COM7" -ForegroundColor Cyan
    exit 0
}

# Validate board selection
if (!$boards.ContainsKey($Board)) {
    Write-Host "ERROR: Board '$Board' not found!" -ForegroundColor Red
    Write-Host "`nAvailable boards:" -ForegroundColor Yellow
    $boards.Keys | Sort-Object | ForEach-Object {
        Write-Host "  - $_ ($($boards[$_].Name))" -ForegroundColor Green
    }
    exit 1
}

$config = $boards[$Board]
$SKETCH_PATH = $config.Path
$BOARD_FQBN = $config.FQBN
$BUILD_DIR = "build/$Board"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Uploading to: $($config.Name)" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

# Check if Arduino CLI is installed
$arduinoCliPath = Get-Command arduino-cli -ErrorAction SilentlyContinue
if (!$arduinoCliPath) {
    Write-Host "ERROR: Arduino CLI not found!" -ForegroundColor Red
    Write-Host "`nPlease install Arduino CLI:" -ForegroundColor Yellow
    Write-Host "  https://arduino.github.io/arduino-cli/latest/installation/" -ForegroundColor Cyan
    exit 1
}

# Auto-detect port if not specified
if (!$Port) {
    Write-Host "`nAuto-detecting board..." -ForegroundColor Yellow
    $boardList = arduino-cli board list 2>&1
    Write-Host $boardList
    
    # Try to find ESP32 board
    $detectedPort = $boardList | Select-String -Pattern "(COM\d+)" | ForEach-Object { $_.Matches[0].Value } | Select-Object -First 1
    
    if ($detectedPort) {
        $Port = $detectedPort
        Write-Host "Detected board on port: $Port" -ForegroundColor Green
    } else {
        Write-Host "ERROR: No board detected!" -ForegroundColor Red
        Write-Host "`nPlease ensure:" -ForegroundColor Yellow
        Write-Host "  1. The ESP32 board is connected via USB" -ForegroundColor Yellow
        Write-Host "  2. The USB drivers are installed (CP210x or CH340)" -ForegroundColor Yellow
        Write-Host "  3. No other program is using the serial port" -ForegroundColor Yellow
        Write-Host "`nOr specify port manually: .\upload.ps1 -Board $Board -Port COM7" -ForegroundColor Cyan
        exit 1
    }
}

Write-Host "`nPort: $Port" -ForegroundColor Cyan
Write-Host "Board: $($config.Name)" -ForegroundColor Cyan
Write-Host "FQBN: $BOARD_FQBN" -ForegroundColor Cyan

# Handle erase flash if requested
if ($Erase) {
    Write-Host "`nERASING FLASH MEMORY..." -ForegroundColor Yellow
    Write-Host "This will delete all stored configuration and firmware." -ForegroundColor Yellow
    arduino-cli burn-bootloader --fqbn $BOARD_FQBN --port $Port --programmer esptool
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Flash erased successfully!" -ForegroundColor Green
        Write-Host "Now uploading fresh firmware..." -ForegroundColor Cyan
    } else {
        Write-Host "`nFlash erase failed!" -ForegroundColor Red
        exit 1
    }
}

# Check if build exists
$binaryFile = "$BUILD_DIR\$Board.ino.bin"
if (Test-Path $binaryFile) {
    Write-Host "Found existing build" -ForegroundColor Green
    Write-Host "`nUploading to $Port..." -ForegroundColor Yellow
    arduino-cli upload --fqbn $BOARD_FQBN --port $Port --input-dir $BUILD_DIR $SKETCH_PATH
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "`nUpload failed!" -ForegroundColor Red
        Write-Host "`nTroubleshooting tips:" -ForegroundColor Yellow
        Write-Host "  1. Check that the board is connected and powered" -ForegroundColor Yellow
        Write-Host "  2. Try a different USB cable or port" -ForegroundColor Yellow
        Write-Host "  3. Close any serial monitors or other programs using the port" -ForegroundColor Yellow
        Write-Host "  4. Press the BOOT button on the ESP32 during upload (if needed)" -ForegroundColor Yellow
        Write-Host "  5. Verify correct board: .\upload.ps1 -Board $Board" -ForegroundColor Yellow
        Write-Host "  6. Try erasing flash first: .\upload.ps1 -Board $Board -Port $Port -Erase" -ForegroundColor Yellow
        exit 1
    }
} else {
    Write-Host "No existing build found at: $binaryFile" -ForegroundColor Red
    Write-Host "`nPlease build the firmware first:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1 $Board" -ForegroundColor Cyan
    Write-Host "`nOr use compile + upload in one step:" -ForegroundColor Yellow
    Write-Host "  arduino-cli compile --fqbn $BOARD_FQBN --upload --port $Port $SKETCH_PATH" -ForegroundColor Cyan
    exit 1
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Upload successful!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "`nNext steps:" -ForegroundColor Cyan
Write-Host "  1. Open serial monitor: arduino-cli monitor -p $Port -c baudrate=115200" -ForegroundColor Yellow
Write-Host "  2. Connect to device AP: esp32-XXXXXX" -ForegroundColor Yellow
Write-Host "  3. Configure at: http://192.168.4.1" -ForegroundColor Yellow
