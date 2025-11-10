#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include "config_manager.h"
#include "power_manager.h"

// Increase MQTT buffer size for Home Assistant discovery messages
#define MQTT_MAX_PACKET_SIZE 512

// Telemetry data structure for batch publishing
struct TelemetryData {
    // Device info
    String deviceId;
    String deviceName;
    String modelName;
    
    // Wake/power state
    WakeupReason wakeReason;
    
    // Battery metrics (0.0 to skip voltage, -1 to skip percentage)
    float batteryVoltage;
    int batteryPercentage;
    
    // WiFi metrics
    int wifiRSSI;
    String wifiBSSID;  // Empty to skip
    uint8_t wifiRetryCount;  // 255 to skip
    
    // Loop timing
    float loopTimeTotal;      // Total loop time in seconds
    float loopTimeWiFi;       // WiFi connection time (0.0 to skip)
    float loopTimeWork;       // Work/processing time (0.0 to skip)
    
    // Free heap memory in bytes (0 to skip)
    uint32_t freeHeap;
    
    // Constructor with defaults
    TelemetryData() : 
        wakeReason(WAKEUP_FIRST_BOOT),
        batteryVoltage(0.0f),
        batteryPercentage(-1),
        wifiRSSI(0),
        wifiRetryCount(255),
        loopTimeTotal(0.0f),
        loopTimeWiFi(0.0f),
        loopTimeWork(0.0f),
        freeHeap(0) {}
};

class MQTTManager {
public:
    MQTTManager(ConfigManager* configManager);
    ~MQTTManager();
    
    // Initialize MQTT manager (loads config, sets up client)
    bool begin();
    
    // Connect to MQTT broker
    bool connect();
    
    // Disconnect from MQTT broker
    void disconnect();
    
    // Publish Home Assistant auto-discovery configuration
    bool publishDiscovery(const TelemetryData& data);
    
    // Publish individual metrics
    bool publishBatteryVoltage(const String& deviceId, float voltage);
    bool publishBatteryPercentage(const String& deviceId, int percentage);
    bool publishLoopTime(const String& deviceId, float loopTimeSeconds);
    bool publishWiFiSignal(const String& deviceId, int rssi);
    bool publishWiFiBSSID(const String& deviceId, const String& bssid);
    bool publishFreeHeap(const String& deviceId, uint32_t freeHeap);
    
    // Publish all telemetry in a single MQTT session (optimized for battery-powered devices)
    // Connects, publishes discovery (conditionally) + all state messages, then disconnects
    bool publishAllTelemetry(const TelemetryData& data);
    
    // Check if MQTT is configured
    bool isConfigured();
    
    // Get last error message
    String getLastError();
    
private:
    ConfigManager* _configManager;
    WiFiClient _wifiClient;
    PubSubClient* _mqttClient;
    String _broker;
    String _username;
    String _password;
    int _port;
    String _lastError;
    bool _isConfigured;
    
    // Parse broker URL to extract host and port
    bool parseBrokerURL(const String& url, String& host, int& port);
    
    // Generate MQTT topics
    String getDiscoveryTopic(const String& deviceId, const String& sensorType);
    String getStateTopic(const String& deviceId, const String& sensorType);
    
    // Build device info JSON
    String buildDeviceInfoJSON(const String& deviceId, const String& deviceName, const String& modelName, bool full);
    
    // Determine if discovery should be published based on wake reason
    bool shouldPublishDiscovery(WakeupReason wakeReason);
    
    // Helper to publish sensor discovery
    bool publishSensorDiscovery(const String& discoveryTopic, const String& deviceId, const String& sensorType,
                                const String& name, const String& deviceClass, const String& unit,
                                const String& deviceName, const String& modelName, bool includeFullDevice);
};

#endif // MQTT_MANAGER_H
