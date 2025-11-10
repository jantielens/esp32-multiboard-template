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
                       ConfigManager& configManager, PowerManager& powerManager, 
                       float workTime) {
  // Track WiFi connection timing
  unsigned long wifiStartTime = millis();
  uint8_t retryCount = 0;
  
  // Connect to WiFi
  if (!wifiManager.connectToWiFi(&retryCount)) {
    return false;
  }
  
  float wifiTime = (millis() - wifiStartTime) / 1000.0f;  // Convert to seconds
  
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
      telemetry.wifiBSSID = WiFi.BSSIDstr();
      telemetry.wifiRetryCount = retryCount;
      telemetry.loopTimeTotal = millis() / 1000.0f;  // Convert to seconds
      telemetry.loopTimeWiFi = wifiTime;
      telemetry.loopTimeWork = workTime;  // Work time from loop()
      telemetry.freeHeap = ESP.getFreeHeap();

      mqttManager.publishAllTelemetry(telemetry);
      mqttManager.disconnect();
    }
  }
  
  return true;
}

void publishTelemetryAfterWork(WiFiManager& wifiManager, MQTTManager& mqttManager,
                               ConfigManager& configManager, PowerManager& powerManager,
                               float workTime) {
  // Publish telemetry via MQTT (if configured)
  if (mqttManager.begin() && mqttManager.isConfigured()) {
    LogBox::message("MQTT", "Connecting to broker...");

    if (mqttManager.connect()) {
      LogBox::message("MQTT", "Publishing telemetry with work time");

      // Prepare telemetry data
      TelemetryData telemetry;
      telemetry.deviceId = wifiManager.getDeviceIdentifier();
      telemetry.deviceName = configManager.getFriendlyName();
      telemetry.modelName = BOARD_NAME;
      telemetry.wakeReason = powerManager.getWakeupReason();
      telemetry.batteryVoltage = powerManager.readBatteryVoltage();
      telemetry.batteryPercentage = PowerManager::calculateBatteryPercentage(telemetry.batteryVoltage);
      telemetry.wifiRSSI = wifiManager.getRSSI();
      telemetry.wifiBSSID = WiFi.BSSIDstr();
      telemetry.wifiRetryCount = 0;  // Not tracked on republish
      telemetry.loopTimeTotal = millis() / 1000.0f;  // Convert to seconds
      telemetry.loopTimeWiFi = 0.0f;  // Already included in total
      telemetry.loopTimeWork = workTime;
      telemetry.freeHeap = ESP.getFreeHeap();

      mqttManager.publishAllTelemetry(telemetry);
      mqttManager.disconnect();
    }
  }
}

void enterSleepMode(PowerManager& powerManager, ConfigManager& configManager, float sleepDuration) {
  LogBox::messagef("Power", "Entering deep sleep for %.0f seconds", sleepDuration);
  delay(1000); // Give time for message to be sent
  powerManager.enterDeepSleep(sleepDuration);
}
