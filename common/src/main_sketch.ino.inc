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
// 👉 Device Operation Mode Configuration - CUSTOMIZE THIS!
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
  // Check button FIRST before any delays or initialization
  // This must happen immediately to catch button press during boot
  bool forceConfig = checkButtonAtBoot();
  
  // Initialize hardware and core components
  initializeHardware(powerManager, configManager);
  pinMode(LED_PIN, OUTPUT);
  
  // Determine configuration state
  bool isConfigured = configManager.isConfigured();
  
  // Handle first boot scenario - never returns
  if (!isConfigured) {
    handleFirstBoot(apMode);
    return;  // Never reached - config loop runs forever
  }
  
  // Handle reconfiguration scenario - may return after timeout
  if (forceConfig) {
    handleReconfiguration(wifiManager, configPortal, apMode);
    // Falls through if timeout occurred, otherwise reboots or never returns
  }
  
  // Normal operation - connect to WiFi or restart
  LogBox::begin("Configuration");
  LogBox::line("Device is configured");
  LogBox::line("Starting normal operation...");
  LogBox::end();
  
  connectToWiFiOrRestart(wifiManager);
  
  LogBox::message("Setup", "Device ready");
}

// =============================================================================
// Main Loop Function
// =============================================================================

void loop() {
  // ===========================================================================
  // YOUR CUSTOM CODE BELOW - Add your application logic below:
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

  // 👉 TODO: Replace with your actual work
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

  delay(20000); // 👉 Wait 20 seconds before next loop iteration, only for always-on mode
}