#include "startup_helpers.h"
#include "logger.h"
#include "board_config.h"

bool checkButtonAtBoot() {
  pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);
  delay(50); // Short delay to let pullup stabilize
  return (digitalRead(WAKE_BUTTON_PIN) == LOW);
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
  // Track loop timing for continuous operation mode
  static unsigned long lastLoopStartTime = 0;
  unsigned long currentTime = millis();
  
  // Calculate actual loop time (time since last loop started)
  float actualLoopTime;
  if (lastLoopStartTime == 0) {
    // First loop - use time since boot
    actualLoopTime = currentTime / 1000.0f;
  } else {
    // Subsequent loops - use time since last loop
    actualLoopTime = (currentTime - lastLoopStartTime) / 1000.0f;
  }
  lastLoopStartTime = currentTime;
  
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
      telemetry.loopTimeTotal = actualLoopTime;  // Actual loop iteration time
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

void initializeHardware(PowerManager& powerManager, ConfigManager& configManager) {
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
}

void handleFirstBoot(APModeController& apMode) {
  LogBox::message("Config Mode", "First boot - no configuration found");
  enterConfigMode(apMode, "First boot - device needs initial configuration");
  // Never returns - config loop runs forever until configured
}

void handleReconfiguration(WiFiManager& wifiManager, ConfigPortal& configPortal, APModeController& apMode) {
  LogBox::message("Config Mode", "Button held during boot - reconfiguring device");
  
  // Try to connect to existing WiFi first
  uint8_t retryCount = 0;
  if (wifiManager.connectToWiFi(&retryCount)) {
    // Connected - start config portal on WiFi network
    LogBox::message("WiFi", "Connected at " + wifiManager.getLocalIP());
    
    configPortal.begin();
    
    // Run portal until config received or timeout
    unsigned long startTime = millis();
    const unsigned long portalTimeout = 300000; // 5 minutes
    
    while (!configPortal.isConfigReceived() && (millis() - startTime < portalTimeout)) {
      configPortal.handleClient();
      delay(10);
    }
    
    if (configPortal.isConfigReceived()) {
      LogBox::message("Config", "New configuration received - rebooting");
      delay(2000);
      ESP.restart();
    } else {
      LogBox::message("Config", "Timeout - continuing normal operation");
      configPortal.stop();
    }
  } else {
    // WiFi connection failed - fall back to AP mode
    LogBox::message("WiFi", "Connection failed - starting AP mode");
    enterConfigMode(apMode, "Reconfiguring device (AP mode fallback)");
    // Never returns if AP mode succeeds
  }
}

void connectToWiFiOrRestart(WiFiManager& wifiManager) {
  LogBox::begin("WiFi Connection");
  LogBox::line("Connecting to saved network...");
  LogBox::end();
  
  uint8_t retryCount = 0;
  if (!wifiManager.connectToWiFi(&retryCount)) {
    // WiFi failed - reboot to retry (avoids getting stuck)
    LogBox::message("WiFi", "Failed to connect to saved network");
    LogBox::message("Reboot", "Rebooting in 5 seconds to retry...");
    delay(5000);
    ESP.restart();
  }
  
  LogBox::message("WiFi", "Connected successfully");
  LogBox::message("WiFi", "IP: " + wifiManager.getLocalIP());
  LogBox::messagef("WiFi", "RSSI: %d dBm", wifiManager.getRSSI());
}
