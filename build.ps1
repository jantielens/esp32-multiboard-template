# Arduino CLI Build Script for ESP32 Multi-Board Template

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet('esp32_dev', 'esp32s3_dev', 'all')]
    [string]$Board = "esp32_dev"
)

$WORKSPACE_PATH = (Get-Location).Path
$COMMON_PATH = Join-Path $WORKSPACE_PATH "common"

Write-Host "ESP32 Multi-Board Template - Build Script" -ForegroundColor Cyan

$boards = @{
    'esp32_dev' = @{
        Name = "ESP32 DevKit V1"
        FQBN = "esp32:esp32:esp32"
        Path = "boards/esp32_dev"
    }
    'esp32s3_dev' = @{
        Name = "ESP32-S3 DevKit"
        FQBN = "esp32:esp32:esp32s3"
        Path = "boards/esp32s3_dev"
    }
}

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
    
    Write-Host "Compiling..." -ForegroundColor Yellow
    
    $BOARD_CONFIG_PATH = "$WORKSPACE_PATH\$SKETCH_PATH"
    
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

$esp32Core = & $arduinoCliPath core list | Select-String "esp32:esp32"
if (!$esp32Core) {
    Write-Host "Installing ESP32 core..." -ForegroundColor Yellow
    & $arduinoCliPath core update-index
    & $arduinoCliPath core install esp32:esp32
}


# Check required libraries
$requiredLibs = @('PubSubClient')
foreach ($lib in $requiredLibs) {
    $libInstalled = & $arduinoCliPath lib list | Select-String $lib
    if (!$libInstalled) {
        Write-Host "Installing required library: $lib..." -ForegroundColor Yellow
        & $arduinoCliPath lib install $lib
    }
}

if ($Board -eq 'all') {
    foreach ($key in $boards.Keys) {
        Build-Board -BoardKey $key
    }
} else {
    Build-Board -BoardKey $Board
}

Write-Host "Build complete" -ForegroundColor Green

