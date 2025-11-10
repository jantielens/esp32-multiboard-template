#ifndef STARTUP_HELPERS_H
#define STARTUP_HELPERS_H

#include "power_manager.h"
#include "config_manager.h"
#include "wifi_manager.h"
#include "config_portal.h"
#include "ap_mode_controller.h"
#include "mqtt_manager.h"

/**
 * @brief Check if user is holding button to force config mode
 * @param powerManager Reference to power manager instance
 * @return true if button is pressed during boot
 */
bool checkForceConfigMode(PowerManager& powerManager);

/**
 * @brief Enter configuration mode and wait for user input
 * @param apMode Reference to AP mode controller
 * @param reason Why we're entering config mode (e.g., "First Boot", "Button Press", "WiFi Failed")
 */
void enterConfigMode(APModeController& apMode, const char* reason);

/**
 * @brief Connect to WiFi and publish MQTT telemetry
 * @param wifiManager Reference to WiFi manager
 * @param mqttManager Reference to MQTT manager
 * @param configManager Reference to config manager
 * @param powerManager Reference to power manager
 * @param workTime Optional work time in seconds (0.0 if not available yet)
 * @return true if WiFi connected successfully
 */
bool connectAndPublish(WiFiManager& wifiManager, MQTTManager& mqttManager, 
                       ConfigManager& configManager, PowerManager& powerManager, 
                       float workTime = 0.0f);

/**
 * @brief Publish MQTT telemetry after work is complete (for battery mode)
 * Call this after your custom work in loop() before entering sleep
 * @param wifiManager Reference to WiFi manager
 * @param mqttManager Reference to MQTT manager
 * @param configManager Reference to config manager
 * @param powerManager Reference to power manager
 * @param workTime Work time in seconds
 */
void publishTelemetryAfterWork(WiFiManager& wifiManager, MQTTManager& mqttManager,
                               ConfigManager& configManager, PowerManager& powerManager,
                               float workTime);

/**
 * @brief Enter deep sleep mode to save power
 * @param powerManager Reference to power manager
 * @param configManager Reference to config manager
 * @param sleepDuration Sleep duration in seconds
 */
void enterSleepMode(PowerManager& powerManager, ConfigManager& configManager, float sleepDuration);

#endif // STARTUP_HELPERS_H
