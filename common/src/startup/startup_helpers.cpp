#include "startup_helpers.h"
#include "logger.h"
#include "board_config.h"

bool checkForceConfigMode(PowerManager& powerManager) {
  pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);
  bool forced = (digitalRead(WAKE_BUTTON_PIN) == LOW);
  
  if (forced) {
    LogBox::message("Config Mode", "Button held - entering configuration mode");
    delay(500); // Debounce
  }
  
  return forced;
}

void enterConfigMode(APModeController& apMode, const char* reason) {
  LogBox::message("Config Mode", reason);
  
  if (apMode.begin()) {
    apMode.runConfigurationLoop();
  } else {
    LogBox::message("Error", "Failed to start config mode");
  }
}

bool connectAndPublish(WiFiManager& wifiManager, MQTTManager& mqttManager, 
                       ConfigManager& configManager, PowerManager& powerManager) {
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
      telemetry.batteryVoltage = powerManager.readBatteryVoltage();
      telemetry.batteryPercentage = PowerManager::calculateBatteryPercentage(telemetry.batteryVoltage);
      telemetry.wifiRSSI = wifiManager.getRSSI();
      telemetry.loopTimeTotal = millis();
      telemetry.freeHeap = ESP.getFreeHeap();

      mqttManager.publishAllTelemetry(telemetry);
      mqttManager.disconnect();
    }
  }
  
  return true;
}

void enterSleepMode(PowerManager& powerManager, ConfigManager& configManager, float sleepDuration) {
  LogBox::messagef("Power", "Entering deep sleep for %.0f seconds", sleepDuration);
  delay(1000); // Give time for message to be sent
  powerManager.enterDeepSleep(sleepDuration);
}
