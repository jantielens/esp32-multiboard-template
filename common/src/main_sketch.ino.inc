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
#include "startup_helpers.h"

#define RUN_ONCE_THEN_SLEEP 1
#define RUN_CONTINUOUSLY 2

// =============================================================================
// Device Operation Mode Configuration
// =============================================================================
// Select your device's loop behavior:
// - RUN_ONCE_THEN_SLEEP: Loop runs once per wake cycle, then device enters deep sleep
//                        (Optimized for battery-powered devices)
// - RUN_CONTINUOUSLY: Loop runs repeatedly forever, device never sleeps
//                     (For always-on/mains-powered devices)
#define LOOP_BEHAVIOR RUN_CONTINUOUSLY
// OR #define LOOP_BEHAVIOR RUN_ONCE_THEN_SLEEP


// =============================================================================
// Global Component Instances
// =============================================================================

PowerManager powerManager;
ConfigManager configManager;
WiFiManager wifiManager(&configManager);
MQTTManager mqttManager(&configManager);
ConfigPortal configPortal(&configManager, &wifiManager, &powerManager, &mqttManager);
APModeController apMode(&wifiManager, &configPortal);

// =============================================================================
// Main Setup Function
// =============================================================================

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);

  // Display startup banner
  LogBox::begin("ESP32 Multi-Board Template");
  LogBox::line("Board: " BOARD_NAME);
  LogBox::end();

  // Initialize core components
  LogBox::begin("Initialization");
  LogBox::line("Starting power manager...");
  powerManager.begin(WAKE_BUTTON_PIN);
  LogBox::line("Starting config manager...");
  configManager.begin();
  LogBox::end();

  // Determine operation mode
  bool forceConfig = checkForceConfigMode(powerManager);
  bool isConfigured = configManager.isConfigured();

  if (!isConfigured) {
    // First boot - need initial configuration
    enterConfigMode(apMode, "First boot - no configuration found");

  } else if (forceConfig) {
    // User wants to reconfigure
    enterConfigMode(apMode, "Reconfiguring device");

  } else {
    // Normal operation - connect to WiFi
    LogBox::begin("Configuration");
    LogBox::line("Device is configured");
    LogBox::line("Starting normal operation...");
    LogBox::end();

    // Connect to WiFi (MQTT publishing will happen in loop after work)
    uint8_t retryCount = 0;
    if (!wifiManager.connectToWiFi(&retryCount)) {
      // WiFi failed - enter config mode for reconfiguration
      LogBox::message("WiFi", "Failed to connect to saved network");
      enterConfigMode(apMode, "WiFi connection failed - reconfiguring");
    } else {
      LogBox::message("WiFi", "Connected successfully");
      LogBox::message("WiFi", "IP: " + wifiManager.getLocalIP());
      LogBox::messagef("WiFi", "RSSI: %d dBm", wifiManager.getRSSI());
    }
  }

  LogBox::message("Setup", "Device ready");
}

// =============================================================================
// Main Loop Function
// =============================================================================

void loop() {
  // ===========================================================================
  // YOUR CUSTOM CODE HERE - Add your application logic below:
  //
  // This loop() function behavior depends on LOOP_BEHAVIOR configuration:
  //
  // RUN_ONCE_THEN_SLEEP (Battery-Powered Mode):
  //   - Device wakes up from deep sleep
  //   - setup() runs (connects to WiFi)
  //   - loop() runs ONCE:
  //     * Performs your custom work
  //     * Publishes MQTT telemetry
  //     * Enters deep sleep for SLEEP_DURATION_SECONDS
  //   - Device sleeps until next wake cycle
  //
  // RUN_CONTINUOUSLY (Always-On Mode):
  //   - setup() runs once (connects to WiFi)
  //   - loop() runs REPEATEDLY forever:
  //     * Performs your custom work
  //     * Publishes MQTT telemetry every iteration
  //     * Recommend adding delay() to avoid flooding MQTT broker
  // ===========================================================================

  // Track work time for telemetry
  unsigned long workStartTime = millis();

  LogBox::begin("Custom Work");
  LogBox::line("Performing application logic...");

  // TODO: Replace with your actual work
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  LogBox::line("Work completed successfully");
  LogBox::end();

  // Calculate work time for telemetry
  float workTime = (millis() - workStartTime) / 1000.0f;  // Convert to seconds

  // Publish MQTT telemetry
  publishTelemetryAfterWork(wifiManager, mqttManager, configManager, powerManager, workTime);

#if LOOP_BEHAVIOR == RUN_ONCE_THEN_SLEEP
  // Battery-powered mode: Enter deep sleep after publishing telemetry
  enterSleepMode(powerManager, configManager, 60);
#endif

  delay(20000); // Wait 20 seconds before next loop iteration, only for always-on mode
}