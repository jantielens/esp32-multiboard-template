// ESP32 Multi-Board Template - Shared Main Sketch
// This file contains the shared setup() and loop() implementation
// It should be included from board-specific .ino files after board validation
// Assumes board_config.h has already been included by the .ino file

#include <src/logging/logger.h>
#include <src/power/power_manager.h>
#include <src/config/config_manager.h>
#include <src/wifi/wifi_manager.h>
#include <src/portal/config_portal.h>
#include <src/modes/ap_mode_controller.h>
#include <src/mqtt/mqtt_manager.h>

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
}

void loop() {
  // Blink LED to show we're running
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
