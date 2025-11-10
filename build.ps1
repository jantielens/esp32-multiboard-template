# Arduino CLI Build Script for ESP32 Multi-Board Template

param(
    [Parameter(Mandatory=$false)]
    [string]$Board = "all"
)

$WORKSPACE_PATH = (Get-Location).Path
$COMMON_PATH = Join-Path $WORKSPACE_PATH "common"
$BOARDS_PATH = Join-Path $WORKSPACE_PATH "boards"

Write-Host "ESP32 Multi-Board Template - Build Script" -ForegroundColor Cyan

# Function to load board configurations dynamically
function Get-BoardConfigurations {
    $boards = @{}
    
    if (!(Test-Path $BOARDS_PATH)) {
        Write-Host "ERROR: boards/ directory not found" -ForegroundColor Red
        exit 1
    }
    
    $boardDirs = Get-ChildItem -Path $BOARDS_PATH -Directory
    
    if ($boardDirs.Count -eq 0) {
        Write-Host "ERROR: No board directories found in boards/" -ForegroundColor Red
        exit 1
    }
    
    foreach ($dir in $boardDirs) {
        $boardKey = $dir.Name
        $boardJsonPath = Join-Path $dir.FullName "board.json"
        
        if (!(Test-Path $boardJsonPath)) {
            Write-Host "WARNING: Skipping $boardKey - no board.json found" -ForegroundColor Yellow
            continue
        }
        
        try {
            $boardConfig = Get-Content $boardJsonPath -Raw | ConvertFrom-Json
            
            if (!$boardConfig.name -or !$boardConfig.fqbn) {
                Write-Host "WARNING: Skipping $boardKey - board.json missing 'name' or 'fqbn'" -ForegroundColor Yellow
                continue
            }
            
            $boards[$boardKey] = @{
                Name = $boardConfig.name
                FQBN = $boardConfig.fqbn
                Path = "boards/$boardKey"
                BoardManagerUrl = $boardConfig.board_manager_url
            }
            
            Write-Host "Discovered board: $boardKey ($($boardConfig.name))" -ForegroundColor Gray
        }
        catch {
            Write-Host "WARNING: Skipping $boardKey - invalid board.json: $_" -ForegroundColor Yellow
            continue
        }
    }
    
    if ($boards.Count -eq 0) {
        Write-Host "ERROR: No valid boards found. Each board needs a board.json file." -ForegroundColor Red
        exit 1
    }
    
    return $boards
}

# Load board configurations dynamically
$boards = Get-BoardConfigurations

function Build-Board {
    param([string]$BoardKey)
    
    $config = $boards[$BoardKey]
    $SKETCH_PATH = $config.Path
    $BOARD_FQBN = $config.FQBN
    $BUILD_DIR = "build/$BoardKey"
    
    Write-Host "Building: $($config.Name)" -ForegroundColor Green
    
    if (!(Test-Path $BUILD_DIR)) {
        New-Item -ItemType Directory -Path $BUILD_DIR -Force | Out-Null
    }
    
    $commonCppFiles = Get-ChildItem -Path (Join-Path $COMMON_PATH "src") -Filter "*.cpp" -Recurse
    $commonHFiles = Get-ChildItem -Path (Join-Path $COMMON_PATH "src") -Filter "*.h" -Recurse
    $commonIncFiles = Get-ChildItem -Path (Join-Path $COMMON_PATH "src") -Filter "*.ino.inc" -Recurse
    $allCommonFiles = $commonCppFiles + $commonHFiles + $commonIncFiles
    
    foreach ($file in $allCommonFiles) {
        $destPath = Join-Path $SKETCH_PATH $file.Name
        Copy-Item -Path $file.FullName -Destination $destPath -Force
        Write-Host "Copied $($file.Name)" -ForegroundColor Gray
    }
    
    # Update the copied package_config.h with values from package.json
    $copiedPackageConfigPath = Join-Path $SKETCH_PATH "package_config.h"
    if (Test-Path $copiedPackageConfigPath) {
        $packageJsonPath = Join-Path $WORKSPACE_PATH "package.json"
        if (Test-Path $packageJsonPath) {
            try {
                $packageJson = Get-Content $packageJsonPath -Raw | ConvertFrom-Json
                $content = Get-Content $copiedPackageConfigPath -Raw
                
                if ($packageJson.name) {
                    $content = $content -replace '#define PACKAGE_NAME ".*"', "#define PACKAGE_NAME `"$($packageJson.name)`""
                }
                if ($packageJson.displayName) {
                    $content = $content -replace '#define PACKAGE_DISPLAY_NAME ".*"', "#define PACKAGE_DISPLAY_NAME `"$($packageJson.displayName)`""
                }
                if ($packageJson.displayNameShort) {
                    $content = $content -replace '#define PACKAGE_DISPLAY_NAME_SHORT ".*"', "#define PACKAGE_DISPLAY_NAME_SHORT `"$($packageJson.displayNameShort)`""
                }
                
                Set-Content $copiedPackageConfigPath $content -NoNewline
                Write-Host "Updated copied package_config.h with values from package.json" -ForegroundColor Gray
            }
            catch {
                Write-Host "WARNING: Could not update package_config.h from package.json" -ForegroundColor Yellow
            }
        }
    }
    
    Write-Host "Compiling..." -ForegroundColor Yellow
    
    $BOARD_CONFIG_PATH = "$WORKSPACE_PATH\$SKETCH_PATH"
    
    # Arduino CLI now uses build cache automatically with just --build-path
    & $arduinoCliPath compile `
        --fqbn "$BOARD_FQBN" `
        --build-path "$BUILD_DIR" `
        --library "$COMMON_PATH" `
        --build-property "compiler.cpp.extra_flags=-I$COMMON_PATH -I$COMMON_PATH\src -I$BOARD_CONFIG_PATH -include board_config.h" `
        "$SKETCH_PATH"
    
    $BUILD_RESULT = $LASTEXITCODE
    
    foreach ($file in $allCommonFiles) {
        $destPath = Join-Path $SKETCH_PATH $file.Name
        if (Test-Path $destPath) {
            Remove-Item $destPath -Force
        }
    }
    
    if ($BUILD_RESULT -eq 0) {
        Write-Host "Build successful" -ForegroundColor Green
        $binFile = Join-Path $BUILD_DIR "$BoardKey.ino.bin"
        if (Test-Path $binFile) {
            $size = (Get-Item $binFile).Length
            $sizeKB = [math]::Round($size / 1KB, 2)
            Write-Host "Firmware size: $sizeKB KB" -ForegroundColor Cyan
        }
    } else {
        Write-Host "Build failed" -ForegroundColor Red
        exit 1
    }
}

$arduinoCliPath = "C:\Program Files\Arduino CLI\arduino-cli.exe"
if (!(Test-Path $arduinoCliPath)) {
    $arduinoCli = Get-Command arduino-cli -ErrorAction SilentlyContinue
    if ($arduinoCli) {
        $arduinoCliPath = $arduinoCli.Source
    } else {
        Write-Host "ERROR: Arduino CLI not installed" -ForegroundColor Red
        exit 1
    }
}

# Discover and configure board manager URLs
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Discovering Board Manager URLs..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Collect unique board manager URLs from boards being built
$boardManagerUrls = @()
if ($Board -eq "all") {
    # Collect from all boards
    foreach ($boardConfig in $boards.Values) {
        if ($boardConfig.BoardManagerUrl -and $boardManagerUrls -notcontains $boardConfig.BoardManagerUrl) {
            $boardManagerUrls += $boardConfig.BoardManagerUrl
        }
    }
} else {
    # Collect from specific board
    if ($boards.ContainsKey($Board)) {
        $boardConfig = $boards[$Board]
        if ($boardConfig.BoardManagerUrl) {
            $boardManagerUrls += $boardConfig.BoardManagerUrl
        }
    }
}

# Add discovered URLs to arduino-cli config
$urlsAdded = $false
foreach ($url in $boardManagerUrls) {
    Write-Host "Adding board manager URL: $url" -ForegroundColor Gray
    & $arduinoCliPath config add board_manager.additional_urls $url 2>$null
    if ($LASTEXITCODE -eq 0) {
        $urlsAdded = $true
    } else {
        Write-Host "  (URL may already exist, continuing...)" -ForegroundColor DarkGray
    }
}

# Update core index only if new URLs were added or index doesn't exist
if ($urlsAdded -or !(Test-Path "$env:LOCALAPPDATA\Arduino15\package_*_index.json")) {
    Write-Host "Updating core index..." -ForegroundColor Cyan
    & $arduinoCliPath core update-index
} else {
    Write-Host "Core index up to date (skipping update)" -ForegroundColor Gray
}

# Install cores based on FQBNs (extract package prefix from fqbn)
Write-Host "" -ForegroundColor White
Write-Host "Checking Core Installations..." -ForegroundColor Cyan

$coresToInstall = @{}
if ($Board -eq "all") {
    foreach ($boardConfig in $boards.Values) {
        # Extract core package from FQBN (format: package:arch:board)
        if ($boardConfig.FQBN -match '^([^:]+:[^:]+)') {
            $corePackage = $matches[1]
            $coresToInstall[$corePackage] = $true
        }
    }
} else {
    if ($boards.ContainsKey($Board)) {
        $boardConfig = $boards[$Board]
        if ($boardConfig.FQBN -match '^([^:]+:[^:]+)') {
            $corePackage = $matches[1]
            $coresToInstall[$corePackage] = $true
        }
    }
}

# Install each unique core
foreach ($core in $coresToInstall.Keys) {
    # Check if core is installed (any version)
    $coreInstalled = & $arduinoCliPath core list | Select-String "^$core"
    
    if (!$coreInstalled) {
        Write-Host "Core $core not installed. Installing..." -ForegroundColor Yellow
        
        # For esp32:esp32, pin to specific version for consistency
        if ($core -eq "esp32:esp32") {
            & $arduinoCliPath core install "esp32:esp32@3.3.2"
        } else {
            # For other cores, install latest version
            & $arduinoCliPath core install $core
        }
    } else {
        Write-Host "Core $core already installed" -ForegroundColor Green
    }
}

# Check required libraries
Write-Host "" -ForegroundColor White
Write-Host "Checking Library Installation..." -ForegroundColor Cyan

# Read required libraries from libraries.txt
if (Test-Path "libraries.txt") {
    $requiredLibs = Get-Content "libraries.txt" | Where-Object { 
        $_ -notmatch '^\s*#' -and $_ -notmatch '^\s*$' 
    } | ForEach-Object { $_.Trim() }
    
    foreach ($lib in $requiredLibs) {
        Write-Host "Checking library: $lib" -ForegroundColor Cyan
        $libInstalled = & $arduinoCliPath lib list | Select-String $lib
        if (!$libInstalled) {
            Write-Host "Installing required library: $lib..." -ForegroundColor Yellow
            & $arduinoCliPath lib install $lib
            Write-Host "Library $lib installed successfully" -ForegroundColor Green
        } else {
            Write-Host "Library $lib already installed" -ForegroundColor Green
        }
    }
} else {
    Write-Host "No libraries.txt file found, skipping library installation" -ForegroundColor Yellow
}

if ($Board -eq 'all') {
    foreach ($key in $boards.Keys) {
        Build-Board -BoardKey $key
    }
}
elseif ($boards.ContainsKey($Board)) {
    Build-Board -BoardKey $Board
}
else {
    Write-Host "ERROR: Unknown board '$Board'" -ForegroundColor Red
    Write-Host "Available boards: $($boards.Keys -join ', ')" -ForegroundColor Yellow
    exit 1
}

Write-Host "Build complete" -ForegroundColor Green

