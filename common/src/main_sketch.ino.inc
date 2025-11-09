// ESP32 Multi-Board Template - Shared Main Sketch
// This file contains the shared setup() and loop() implementation
// It should be included from board-specific .ino files after board validation
// Assumes board_config.h has already been included by the .ino file

#include "logger.h"
#include "power_manager.h"
#include "config_manager.h"
#include "wifi_manager.h"
#include "config_portal.h"
#include "ap_mode_controller.h"
#include "mqtt_manager.h"

// Global component instances
PowerManager powerManager;
ConfigManager configManager;
WiFiManager wifiManager(&configManager);
MQTTManager mqttManager(&configManager);
ConfigPortal configPortal(&configManager, &wifiManager, &powerManager, &mqttManager);
APModeController apMode(&wifiManager, &configPortal);

// Forward declarations
bool checkForceConfigMode();
void enterConfigMode(const char* reason);
bool connectAndPublish();
void enterSleepMode();

// =============================================================================
// CUSTOM WORK - Start here for your application logic
// =============================================================================

/**
 * @brief Perform custom application work
 * 
 * TODO: Replace this function with your device-specific functionality:
 * - Read sensors (temperature, humidity, light, etc.)
 * - Control actuators (relays, motors, servos, etc.)
 * - Display information on screens
 * - Capture and process images
 * - Perform calculations or data processing
 */
void performCustomWork() {
  LogBox::message("Work", "Performing custom work...");
  
  // Simulate work with a delay (replace with your actual logic)
  delay(2000);
  
  LogBox::message("Work", "Custom work completed");
}

// =============================================================================
// Main Setup & Loop
// =============================================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  LogBox::begin("ESP32 Multi-Board Template");
  LogBox::line("Board: " BOARD_NAME);
  LogBox::end();

  // Initialize components
  powerManager.begin(WAKE_BUTTON_PIN);
  configManager.begin();

  // Check if we need to enter config mode
  bool forceConfig = checkForceConfigMode();
  bool isConfigured = configManager.isConfigured();

  if (!isConfigured) {
    // First boot - need initial configuration
    enterConfigMode("First boot - no configuration found");
  } else if (forceConfig) {
    // User wants to reconfigure
    enterConfigMode("Reconfiguring device");
  } else {
    // Normal operation - connect and work
    LogBox::message("Config", "Device is configured");
    
    if (!connectAndPublish()) {
      // WiFi failed - enter config mode for reconfiguration
      LogBox::message("WiFi", "Failed to connect to saved network");
      enterConfigMode("WiFi connection failed - reconfiguring");
    }
  }

  LogBox::message("Setup Complete", "Device ready");

  // Perform application-specific work
  performCustomWork();

  // Enter deep sleep to save power
  enterSleepMode();
}

void loop() {
  // This loop only runs if deep sleep is not available (USB-powered devices)
  // Blink LED to show we're running
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}

// =============================================================================
// Helper Functions (Internal - don't modify unless you know what you're doing)
// =============================================================================

/**
 * @brief Check if user is holding button to force config mode
 * @return true if button is pressed during boot
 */
bool checkForceConfigMode() {
  pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);
  bool forced = (digitalRead(WAKE_BUTTON_PIN) == LOW);
  
  if (forced) {
    LogBox::message("Config Mode", "Button held - entering configuration mode");
    delay(500); // Debounce
  }
  
  return forced;
}

/**
 * @brief Enter configuration mode and wait for user input
 * @param reason Why we're entering config mode (e.g., "First Boot", "Button Press", "WiFi Failed")
 */
void enterConfigMode(const char* reason) {
  LogBox::message("Config Mode", reason);
  
  if (apMode.begin()) {
    apMode.runConfigurationLoop();
  } else {
    LogBox::message("Error", "Failed to start config mode");
  }
}

/**
 * @brief Connect to WiFi and publish MQTT telemetry
 * @return true if WiFi connected successfully
 */
bool connectAndPublish() {
  // Connect to WiFi
  if (!wifiManager.connectToWiFi()) {
    return false;
  }
  
  LogBox::message("WiFi", "Connected successfully");
  LogBox::message("WiFi", "IP: " + wifiManager.getLocalIP());
  LogBox::messagef("WiFi", "RSSI: %d dBm", wifiManager.getRSSI());

  // Publish telemetry via MQTT (if configured)
  if (mqttManager.begin() && mqttManager.isConfigured()) {
    LogBox::message("MQTT", "Connecting to broker...");

    if (mqttManager.connect()) {
      LogBox::message("MQTT", "Publishing telemetry");

      // Prepare telemetry data
      TelemetryData telemetry;
      telemetry.deviceId = wifiManager.getDeviceIdentifier();
      telemetry.deviceName = configManager.getFriendlyName();
      telemetry.modelName = BOARD_NAME;
      telemetry.wakeReason = powerManager.getWakeupReason();

      #if HAS_BATTERY
      telemetry.batteryVoltage = powerManager.readBatteryVoltage();
      telemetry.batteryPercentage = PowerManager::calculateBatteryPercentage(telemetry.batteryVoltage);
      #endif

      telemetry.wifiRSSI = wifiManager.getRSSI();
      telemetry.loopTimeTotal = millis();
      telemetry.freeHeap = ESP.getFreeHeap();

      mqttManager.publishAllTelemetry(telemetry);
      mqttManager.disconnect();
    }
  }
  
  return true;
}

/**
 * @brief Enter deep sleep mode to save power
 */
void enterSleepMode() {
  #if HAS_BATTERY
  uint32_t sleepDuration = configManager.getSleepDuration();
  LogBox::messagef("Power", "Entering deep sleep for %d seconds", sleepDuration);
  delay(1000); // Give time for message to be sent
  
  powerManager.enterDeepSleep(sleepDuration);
  #else
  // No battery - stay awake
  LogBox::message("Power", "No battery - staying awake");
  #endif
}