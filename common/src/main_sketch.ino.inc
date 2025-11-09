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

void setup() {
  Serial.begin(115200);
  delay(1000);

  LogBox::begin("ESP32 Multi-Board Template");
  LogBox::line("Board: " BOARD_NAME);
  LogBox::line("All Components Demo");
  LogBox::end();

  // Initialize components
  powerManager.begin(WAKE_BUTTON_PIN);
  configManager.begin();

  // Check if configured
  if (!configManager.isConfigured()) {
    LogBox::message("First Boot", "Starting AP mode for configuration");

    if (apMode.begin()) {
      LogBox::message("AP Mode", "Connect to: " + apMode.getAPName());
      LogBox::message("AP Mode", "Configure at: http://" + apMode.getAPIP());

      // Wait for configuration
      while (!apMode.isConfigReceived()) {
        apMode.handleClient();
        delay(10);
      }

      LogBox::message("AP Mode", "Configuration received, rebooting...");
      delay(2000);
      ESP.restart();
    }
  } else {
    LogBox::message("Config", "Device is configured");

    // Connect to WiFi
    if (wifiManager.connectToWiFi()) {
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
    }
  }

  LogBox::message("Setup Complete", "Device ready");
  
  // ==========================================================================
  // TODO: Add your custom application logic here
  // ==========================================================================
  // This is where you implement your device-specific functionality.
  // Examples:
  // - Read sensors (temperature, humidity, light, etc.)
  // - Control actuators (relays, motors, servos, etc.)
  // - Display information on screens
  // - Capture and process images
  // - Perform calculations or data processing
  //
  // For now, we'll simulate some work with a delay:
  
  LogBox::message("Work", "Performing custom work...");
  delay(2000); // Simulate 2 seconds of work
  LogBox::message("Work", "Custom work completed");
  
  // ==========================================================================
  // Enter deep sleep to save power
  // ==========================================================================
  // After completing work, enter deep sleep to conserve battery.
  // The device will wake up based on the configured sleep interval.
  
  #if HAS_BATTERY
  uint32_t sleepDuration = configManager.getSleepDuration();
  LogBox::messagef("Power", "Entering deep sleep for %d seconds", sleepDuration);
  
  powerManager.enterDeepSleep(sleepDuration);
  #else
  // No battery - stay awake and blink LED
  LogBox::message("Power", "Staying awake");
  #endif
}

void loop() {
  // This loop only runs if deep sleep is not supported or disabled
  // Blink LED to show we're running
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
