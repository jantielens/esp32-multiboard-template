#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// ============================================
// BOARD IDENTIFICATION
// ============================================
#define BOARD_NAME "ESP32 DevKit V1"
#define BOARD_TYPE ESP32_DEV

// ============================================
// DISPLAY CONFIGURATION (if applicable)
// ============================================
// Uncomment and configure if board has display
// #define HAS_DISPLAY true
// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64

// ============================================
// POWER MANAGEMENT
// ============================================
#define HAS_BUTTON true              // Wake button present?
#define WAKE_BUTTON_PIN 0            // GPIO0 - Boot button on most ESP32 dev boards
#define HAS_BATTERY false            // Battery monitoring available?
// #define BATTERY_ADC_PIN 35        // GPIO for battery ADC (if HAS_BATTERY)
#define WATCHDOG_TIMEOUT_SECONDS 30  // Watchdog timeout

// ============================================
// BOARD-SPECIFIC PINS
// ============================================
#define LED_PIN 2                    // Built-in LED

// ============================================
// BOARD-SPECIFIC SETTINGS
// ============================================
// Add your custom settings here

#endif // BOARD_CONFIG_H
