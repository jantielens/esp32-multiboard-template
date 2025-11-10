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
    // Normal operation - connect and work
    LogBox::begin("Configuration");
    LogBox::line("Device is configured");
    LogBox::line("Starting normal operation...");
    LogBox::end();

    if (!connectAndPublish(wifiManager, mqttManager, configManager, powerManager)) {
      // WiFi failed - enter config mode for reconfiguration
      LogBox::message("WiFi", "Failed to connect to saved network");
      enterConfigMode(apMode, "WiFi connection failed - reconfiguring");
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
  // This loop() function runs continuously after setup() completes.
  //
  // IMPORTANT: Choose your device's operation mode:
  //
  // MODE 1: BATTERY-POWERED (Sleep/Wake Cycles)
  //   - Add your work code here (read sensors, log data, etc.)
  //   - Uncomment enterSleepMode() at the end of this loop
  //   - Device will: wake → setup() → loop() once → sleep → repeat
  //   - Each wake cycle runs loop() exactly once before sleeping
  //
  // MODE 2: CONTINUOUS OPERATION (Always-On)
  //   - Add your work code here (web server, monitoring, etc.)
  //   - Keep enterSleepMode() commented out
  //   - Device will: setup() → loop() forever (never sleeps)
  //   - Loop runs repeatedly - be mindful of delays and blocking code
  //
  // Examples of what to add:
  //   - Read sensors (temperature, humidity, light, GPS, accelerometer)
  //   - Control actuators (relays, motors, servos, LEDs)
  //   - Update displays (e-ink, LCD, OLED, LED matrices)
  //   - Process data or perform calculations
  //   - Handle web server requests (if running continuous mode)
  //   - Log custom telemetry or events
  // ===========================================================================

  LogBox::begin("Custom Work");
  LogBox::line("Performing application logic...");

  // TODO: Replace with your actual work
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  LogBox::line("Work completed successfully");
  LogBox::end();

  // Uncomment for MODE 1 (battery-powered with sleep/wake cycles):
  // enterSleepMode(powerManager, configManager, 60);  // Sleep for 1 minute

  // If enterSleepMode() is commented out, this loop repeats forever (MODE 2)
}