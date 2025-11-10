#include "mqtt_manager.h"
#include "logger.h"
#include <WiFi.h>

MQTTManager::MQTTManager(ConfigManager* configManager)
    : _configManager(configManager), _mqttClient(nullptr), _port(1883), _isConfigured(false) {
}

MQTTManager::~MQTTManager() {
    disconnect();
    if (_mqttClient != nullptr) {
        delete _mqttClient;
        _mqttClient = nullptr;
    }
}

bool MQTTManager::begin() {
    LogBox::begin("Initializing MQTT Manager");
    
    // Load MQTT configuration
    _broker = _configManager->getMQTTBroker();
    _username = _configManager->getMQTTUsername();
    _password = _configManager->getMQTTPassword();
    
    // Check if MQTT is configured
    if (_broker.length() == 0) {
        LogBox::line("MQTT not configured - skipping");
        LogBox::end();
        _isConfigured = false;
        return true;  // Not an error, just not configured
    }
    
    // Parse broker URL
    String host;
    if (!parseBrokerURL(_broker, host, _port)) {
        _lastError = "Invalid broker URL format";
        LogBox::line("ERROR: " + _lastError);
        LogBox::end();
        _isConfigured = false;
        return false;
    }
    
    LogBox::line("Broker: " + host + ":" + String(_port));
    LogBox::line("Username: " + (_username.length() > 0 ? _username : "(none)"));
    
    // Create MQTT client
    if (_mqttClient == nullptr) {
        _mqttClient = new PubSubClient(_wifiClient);
    }
    
    _mqttClient->setBufferSize(MQTT_MAX_PACKET_SIZE);
    _mqttClient->setServer(host.c_str(), _port);
    _mqttClient->setKeepAlive(5);
    _mqttClient->setSocketTimeout(2);
    
    _isConfigured = true;
    LogBox::end("MQTT Manager initialized successfully");
    
    return true;
}

bool MQTTManager::connect() {
    if (!_isConfigured) {
        return true;  // Not an error
    }
    
    if (_mqttClient == nullptr) {
        _lastError = "MQTT client not initialized";
        return false;
    }
    
    LogBox::begin("Connecting to MQTT broker");
    
    String host;
    int port;
    if (!parseBrokerURL(_broker, host, port)) {
        _lastError = "Failed to parse broker URL";
        LogBox::line("ERROR: " + _lastError);
        LogBox::end();
        return false;
    }
    
    LogBox::line("Broker: " + host + ":" + String(port));
    
    String clientId = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    LogBox::line("Client ID: " + clientId);
    LogBox::line("Auth: " + String(_username.length() > 0 ? "using credentials" : "anonymous"));
    if (_username.length() > 0) {
        LogBox::line("  User: " + _username);
        LogBox::line("  Pass length: " + String(_password.length()));
    }
    
    const int maxRetries = 3;
    bool connected = false;
    
    for (int attempt = 1; attempt <= maxRetries && !connected; attempt++) {
        LogBox::linef("Connection attempt %d/%d...", attempt, maxRetries);
        
        // Force WiFiClient to stop any previous connection
        _wifiClient.stop();
        delay(100);  // Give time for socket to fully close
        
        // Reconnect PubSubClient to server (refreshes internal state)
        _mqttClient->setServer(host.c_str(), port);
        
        if (_username.length() > 0) {
            connected = _mqttClient->connect(clientId.c_str(), _username.c_str(), _password.c_str());
        } else {
            connected = _mqttClient->connect(clientId.c_str());
        }
        
        if (!connected) {
            int state = _mqttClient->state();
            LogBox::line("  Failed with state: " + String(state));
            switch(state) {
                case -4: LogBox::line("  MQTT_CONNECTION_TIMEOUT"); break;
                case -3: LogBox::line("  MQTT_CONNECTION_LOST"); break;
                case -2: LogBox::line("  MQTT_CONNECT_FAILED"); break;
                case -1: LogBox::line("  MQTT_DISCONNECTED"); break;
                case 1: LogBox::line("  MQTT_CONNECT_BAD_PROTOCOL"); break;
                case 2: LogBox::line("  MQTT_CONNECT_BAD_CLIENT_ID"); break;
                case 3: LogBox::line("  MQTT_CONNECT_UNAVAILABLE"); break;
                case 4: LogBox::line("  MQTT_CONNECT_BAD_CREDENTIALS"); break;
                case 5: LogBox::line("  MQTT_CONNECT_UNAUTHORIZED"); break;
            }
            
            if (attempt < maxRetries) {
                delay(500);
            }
        }
    }
    
    if (connected) {
        LogBox::end("Connected to MQTT broker");
        return true;
    } else {
        int finalState = _mqttClient->state();
        _lastError = "Connection failed after " + String(maxRetries) + " attempts (state: " + String(finalState) + ")";
        LogBox::line("ERROR: " + _lastError);
        LogBox::end();
        return false;
    }
}

void MQTTManager::disconnect() {
    if (_mqttClient != nullptr && _mqttClient->connected()) {
        _mqttClient->disconnect();
        LogBox::message("MQTT", "Disconnected from broker");
    }
}

bool MQTTManager::parseBrokerURL(const String& url, String& host, int& port) {
    // Expected formats: "mqtt://hostname:port" or "hostname:port" or "hostname"
    String workUrl = url;
    
    // Remove protocol prefix if present
    if (workUrl.startsWith("mqtt://")) {
        workUrl = workUrl.substring(7);
    } else if (workUrl.startsWith("mqtts://")) {
        workUrl = workUrl.substring(8);
    }
    
    // Split host and port
    int colonPos = workUrl.lastIndexOf(':');
    if (colonPos > 0) {
        host = workUrl.substring(0, colonPos);
        String portStr = workUrl.substring(colonPos + 1);
        port = portStr.toInt();
        if (port == 0) {
            port = 1883;  // Default MQTT port
        }
    } else {
        host = workUrl;
        port = 1883;
    }
    
    return host.length() > 0;
}

String MQTTManager::getDiscoveryTopic(const String& deviceId, const String& sensorType) {
    return "homeassistant/sensor/" + deviceId + "/" + sensorType + "/config";
}

String MQTTManager::getStateTopic(const String& deviceId, const String& sensorType) {
    return "homeassistant/sensor/" + deviceId + "/" + sensorType + "/state";
}

String MQTTManager::buildDeviceInfoJSON(const String& deviceId, const String& deviceName, const String& modelName, bool full) {
    String json = "\"device\":{";
    json += "\"identifiers\":[\"" + deviceId + "\"],";
    json += "\"name\":\"" + deviceName + "\",";
    json += "\"model\":\"" + modelName + "\",";
    json += "\"manufacturer\":\"ESP32\"";
    if (full) {
        json += ",\"sw_version\":\"1.0.0\"";  // Update with actual version
    }
    json += "}";
    return json;
}

bool MQTTManager::shouldPublishDiscovery(WakeupReason wakeReason) {
    // Publish discovery on first boot, button press, or reset
    return (wakeReason == WAKEUP_FIRST_BOOT || 
            wakeReason == WAKEUP_BUTTON || 
            wakeReason == WAKEUP_RESET_BUTTON);
}

bool MQTTManager::publishSensorDiscovery(const String& discoveryTopic, const String& deviceId, const String& sensorType,
                                        const String& name, const String& deviceClass, const String& unit,
                                        const String& deviceName, const String& modelName, bool includeFullDevice) {
    String payload = "{";
    payload += "\"name\":\"" + name + "\",";
    payload += "\"unique_id\":\"" + deviceId + "_" + sensorType + "\",";
    payload += "\"state_topic\":\"" + getStateTopic(deviceId, sensorType) + "\"";
    
    if (deviceClass.length() > 0) {
        payload += ",\"device_class\":\"" + deviceClass + "\"";
    }
    
    if (unit.length() > 0) {
        payload += ",\"unit_of_measurement\":\"" + unit + "\"";
    }
    
    payload += "," + buildDeviceInfoJSON(deviceId, deviceName, modelName, includeFullDevice);
    payload += "}";
    
    return _mqttClient->publish(discoveryTopic.c_str(), payload.c_str(), true);
}

bool MQTTManager::publishDiscovery(const TelemetryData& data) {
    if (!_isConfigured || !_mqttClient->connected()) {
        return false;
    }
    
    LogBox::line("Publishing discovery messages...");
    
    int publishCount = 0;
    
    // Battery voltage sensor
    if (data.batteryVoltage > 0.0f) {
        publishSensorDiscovery(getDiscoveryTopic(data.deviceId, "battery_voltage"), data.deviceId, "battery_voltage",
                              "Battery Voltage", "voltage", "V", data.deviceName, data.modelName, true);
        publishCount++;
    }
    
    // Battery percentage sensor
    if (data.batteryPercentage >= 0) {
        publishSensorDiscovery(getDiscoveryTopic(data.deviceId, "battery_percentage"), data.deviceId, "battery_percentage",
                              "Battery Percentage", "battery", "%", data.deviceName, data.modelName, false);
        publishCount++;
    }
    
    // Loop time sensor
    publishSensorDiscovery(getDiscoveryTopic(data.deviceId, "loop_time"), data.deviceId, "loop_time",
                          "Loop Time", "duration", "s", data.deviceName, data.modelName, false);
    publishCount++;
    
    // WiFi signal sensor
    publishSensorDiscovery(getDiscoveryTopic(data.deviceId, "wifi_signal"), data.deviceId, "wifi_signal",
                          "WiFi Signal", "signal_strength", "dBm", data.deviceName, data.modelName, false);
    publishCount++;
    
    // WiFi BSSID sensor
    if (data.wifiBSSID.length() > 0) {
        publishSensorDiscovery(getDiscoveryTopic(data.deviceId, "wifi_bssid"), data.deviceId, "wifi_bssid",
                              "WiFi BSSID", "", "", data.deviceName, data.modelName, false);
        publishCount++;
    }
    
    // WiFi retry count
    if (data.wifiRetryCount != 255) {
        publishSensorDiscovery(getDiscoveryTopic(data.deviceId, "wifi_retries"), data.deviceId, "wifi_retries",
                              "WiFi Retries", "", "", data.deviceName, data.modelName, false);
        publishCount++;
    }
    
    // Loop time breakdown
    if (data.loopTimeWiFi > 0.0f) {
        publishSensorDiscovery(getDiscoveryTopic(data.deviceId, "loop_time_wifi"), data.deviceId, "loop_time_wifi",
                              "Loop Time - WiFi", "duration", "s", data.deviceName, data.modelName, false);
        publishCount++;
    }
    
    if (data.loopTimeWork > 0.0f) {
        publishSensorDiscovery(getDiscoveryTopic(data.deviceId, "loop_time_work"), data.deviceId, "loop_time_work",
                              "Loop Time - Work", "duration", "s", data.deviceName, data.modelName, false);
        publishCount++;
    }
    
    // Free heap sensor
    if (data.freeHeap > 0) {
        publishSensorDiscovery(getDiscoveryTopic(data.deviceId, "free_heap"), data.deviceId, "free_heap",
                              "Free Heap", "", "bytes", data.deviceName, data.modelName, false);
        publishCount++;
    }
    
    LogBox::linef("Published %d discovery messages", publishCount);
    return true;
}

bool MQTTManager::publishBatteryVoltage(const String& deviceId, float voltage) {
    if (!_isConfigured || !_mqttClient->connected()) return false;
    String topic = getStateTopic(deviceId, "battery_voltage");
    return _mqttClient->publish(topic.c_str(), String(voltage, 2).c_str());
}

bool MQTTManager::publishBatteryPercentage(const String& deviceId, int percentage) {
    if (!_isConfigured || !_mqttClient->connected()) return false;
    String topic = getStateTopic(deviceId, "battery_percentage");
    return _mqttClient->publish(topic.c_str(), String(percentage).c_str());
}

bool MQTTManager::publishLoopTime(const String& deviceId, float loopTimeSeconds) {
    if (!_isConfigured || !_mqttClient->connected()) return false;
    String topic = getStateTopic(deviceId, "loop_time");
    return _mqttClient->publish(topic.c_str(), String(loopTimeSeconds, 2).c_str());
}

bool MQTTManager::publishWiFiSignal(const String& deviceId, int rssi) {
    if (!_isConfigured || !_mqttClient->connected()) return false;
    String topic = getStateTopic(deviceId, "wifi_signal");
    return _mqttClient->publish(topic.c_str(), String(rssi).c_str());
}

bool MQTTManager::publishWiFiBSSID(const String& deviceId, const String& bssid) {
    if (!_isConfigured || !_mqttClient->connected()) return false;
    String topic = getStateTopic(deviceId, "wifi_bssid");
    return _mqttClient->publish(topic.c_str(), bssid.c_str());
}

bool MQTTManager::publishFreeHeap(const String& deviceId, uint32_t freeHeap) {
    if (!_isConfigured || !_mqttClient->connected()) return false;
    String topic = getStateTopic(deviceId, "free_heap");
    return _mqttClient->publish(topic.c_str(), String(freeHeap).c_str());
}

bool MQTTManager::publishAllTelemetry(const TelemetryData& data) {
    if (!_isConfigured) {
        LogBox::message("MQTT", "MQTT not configured - skipping");
        return true;
    }
    
    LogBox::begin("Publishing All Telemetry to MQTT");
    LogBox::line("Connecting to MQTT broker...");
    
    if (!connect()) {
        LogBox::line("ERROR: Failed to connect to MQTT broker");
        LogBox::line("Error: " + _lastError);
        LogBox::end();
        return false;
    }
    
    LogBox::line("Connected successfully");
    
    // Publish discovery messages (conditionally)
    if (shouldPublishDiscovery(data.wakeReason)) {
        publishDiscovery(data);
    } else {
        LogBox::line("Skipping discovery (normal wake cycle)");
    }
    
    // Publish state messages
    LogBox::line("Publishing state messages...");
    int stateCount = 0;
    
    // Publish battery voltage state
    if (data.batteryVoltage > 0.0f) {
        String topic = getStateTopic(data.deviceId, "battery_voltage");
        String payload = String(data.batteryVoltage, 2);
        _mqttClient->publish(topic.c_str(), payload.c_str(), true);
        LogBox::line("Battery: " + payload + " V");
        stateCount++;
    }
    
    // Publish battery percentage state
    if (data.batteryPercentage >= 0) {
        String topic = getStateTopic(data.deviceId, "battery_percentage");
        String payload = String(data.batteryPercentage);
        _mqttClient->publish(topic.c_str(), payload.c_str(), true);
        LogBox::line("Battery %: " + payload + " %");
        stateCount++;
    }
    
    // Publish loop time state
    {
        String topic = getStateTopic(data.deviceId, "loop_time");
        String payload = String(data.loopTimeTotal, 2);
        _mqttClient->publish(topic.c_str(), payload.c_str(), true);
        LogBox::line("Loop Time: " + payload + " s");
        stateCount++;
    }
    
    // Publish WiFi signal state
    {
        String topic = getStateTopic(data.deviceId, "wifi_signal");
        String payload = String(data.wifiRSSI);
        _mqttClient->publish(topic.c_str(), payload.c_str(), true);
        LogBox::line("WiFi Signal: " + payload + " dBm");
        stateCount++;
    }
    
    // Publish WiFi BSSID state
    if (data.wifiBSSID.length() > 0) {
        String topic = getStateTopic(data.deviceId, "wifi_bssid");
        _mqttClient->publish(topic.c_str(), data.wifiBSSID.c_str(), true);
        LogBox::line("WiFi BSSID: " + data.wifiBSSID);
        stateCount++;
    }
    
    // Publish WiFi retry count
    if (data.wifiRetryCount != 255) {
        String topic = getStateTopic(data.deviceId, "wifi_retries");
        String payload = String(data.wifiRetryCount);
        _mqttClient->publish(topic.c_str(), payload.c_str(), true);
        LogBox::line("WiFi Retries: " + payload);
        stateCount++;
    }
    
    // Publish loop time WiFi
    if (data.loopTimeWiFi > 0.0f) {
        String topic = getStateTopic(data.deviceId, "loop_time_wifi");
        String payload = String(data.loopTimeWiFi, 2);
        _mqttClient->publish(topic.c_str(), payload.c_str(), true);
        LogBox::line("Loop Time - WiFi: " + payload + " s");
        stateCount++;
    }
    
    // Publish loop time work
    if (data.loopTimeWork > 0.0f) {
        String topic = getStateTopic(data.deviceId, "loop_time_work");
        String payload = String(data.loopTimeWork, 2);
        _mqttClient->publish(topic.c_str(), payload.c_str(), true);
        LogBox::line("Loop Time - Work: " + payload + " s");
        stateCount++;
    }
    
    // Publish free heap state
    if (data.freeHeap > 0) {
        String topic = getStateTopic(data.deviceId, "free_heap");
        String payload = String(data.freeHeap);
        _mqttClient->publish(topic.c_str(), payload.c_str(), true);
        LogBox::line("Free Heap: " + payload + " bytes");
        stateCount++;
    }
    
    LogBox::linef("Published %d state messages", stateCount);
    
    // Give MQTT client time to transmit all queued messages
    // PubSubClient needs loop() calls to actually send queued data
    // 20-30ms is typically sufficient for transmission
    for (int i = 0; i < 3; i++) {
        _mqttClient->loop();
        delay(10);
    }
    
    // Disconnect
    disconnect();
    
    LogBox::end("MQTT telemetry published successfully");
    return true;
}

bool MQTTManager::isConfigured() {
    return _isConfigured;
}

String MQTTManager::getLastError() {
    return _lastError;
}
