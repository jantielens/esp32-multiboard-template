#include "config_manager.h"
#include "logger.h"

ConfigManager::ConfigManager() : _initialized(false) {
}

ConfigManager::~ConfigManager() {
    if (_initialized) {
        _preferences.end();
    }
}

bool ConfigManager::begin() {
    if (_initialized) {
        return true;
    }
    
    _initialized = _preferences.begin(PREF_NAMESPACE, false);
    if (!_initialized) {
        LogBox::message("ConfigManager Error", "Failed to initialize Preferences");
        return false;
    }
    
    return _initialized;
}

bool ConfigManager::isConfigured() {
    if (!_initialized && !begin()) {
        return false;
    }
    
    return _preferences.getBool(PREF_CONFIGURED, false);
}

bool ConfigManager::hasWiFiConfig() {
    if (!_initialized && !begin()) {
        return false;
    }
    
    String ssid = _preferences.getString(PREF_WIFI_SSID, "");
    return ssid.length() > 0;
}

bool ConfigManager::loadConfig(DeviceConfig& config) {
    if (!_initialized && !begin()) {
        LogBox::message("ConfigManager Error", "ConfigManager not initialized");
        return false;
    }
    
    // Load all configuration values
    config.isConfigured = _preferences.getBool(PREF_CONFIGURED, false);
    
    if (!config.isConfigured) {
        LogBox::message("Config Status", "Device not configured yet");
        return false;
    }
    
    config.wifiSSID = _preferences.getString(PREF_WIFI_SSID, "");
    config.wifiPassword = _preferences.getString(PREF_WIFI_PASS, "");
    config.friendlyName = _preferences.getString(PREF_FRIENDLY_NAME, "");
    config.debugMode = _preferences.getBool(PREF_DEBUG_MODE, false);
    
    // Load static IP configuration
    config.useStaticIP = _preferences.getBool(PREF_USE_STATIC_IP, false);
    config.staticIP = _preferences.getString(PREF_STATIC_IP, "");
    config.gateway = _preferences.getString(PREF_GATEWAY, "");
    config.subnet = _preferences.getString(PREF_SUBNET, "");
    config.primaryDNS = _preferences.getString(PREF_PRIMARY_DNS, "");
    config.secondaryDNS = _preferences.getString(PREF_SECONDARY_DNS, "");
    
    // Load MQTT configuration
    config.mqttBroker = _preferences.getString(PREF_MQTT_BROKER, "");
    config.mqttUsername = _preferences.getString(PREF_MQTT_USER, "");
    config.mqttPassword = _preferences.getString(PREF_MQTT_PASS, "");
    
    // Validate configuration
    if (config.wifiSSID.length() == 0) {
        LogBox::message("Config Error", "Invalid configuration: missing SSID");
        return false;
    }
    
    LogBox::begin("Configuration Loaded");
    LogBox::line("WiFi SSID: " + config.wifiSSID);
    LogBox::line("Friendly Name: " + (config.friendlyName.length() > 0 ? config.friendlyName : "(not set)"));
    if (config.useStaticIP) {
        LogBox::line("Static IP: " + config.staticIP);
    } else {
        LogBox::line("IP Mode: DHCP");
    }
    if (config.mqttBroker.length() > 0) {
        LogBox::line("MQTT Broker: " + config.mqttBroker);
        LogBox::line("MQTT Username: " + (config.mqttUsername.length() > 0 ? config.mqttUsername : "(none)"));
    } else {
        LogBox::line("MQTT: Not configured");
    }
    LogBox::end();
    
    return true;
}

bool ConfigManager::saveConfig(const DeviceConfig& config) {
    if (!_initialized && !begin()) {
        LogBox::message("ConfigManager Error", "ConfigManager not initialized");
        return false;
    }
    
    // Validate input
    if (config.wifiSSID.length() == 0) {
        LogBox::message("Config Error", "WiFi SSID cannot be empty");
        return false;
    }
    
    // Save all configuration values
    _preferences.putString(PREF_WIFI_SSID, config.wifiSSID);
    _preferences.putString(PREF_WIFI_PASS, config.wifiPassword);
    _preferences.putString(PREF_FRIENDLY_NAME, config.friendlyName);
    _preferences.putBool(PREF_DEBUG_MODE, config.debugMode);
    _preferences.putBool(PREF_CONFIGURED, true);
    
    // Save static IP configuration
    _preferences.putBool(PREF_USE_STATIC_IP, config.useStaticIP);
    _preferences.putString(PREF_STATIC_IP, config.staticIP);
    _preferences.putString(PREF_GATEWAY, config.gateway);
    _preferences.putString(PREF_SUBNET, config.subnet);
    _preferences.putString(PREF_PRIMARY_DNS, config.primaryDNS);
    _preferences.putString(PREF_SECONDARY_DNS, config.secondaryDNS);
    
    // Save MQTT configuration
    _preferences.putString(PREF_MQTT_BROKER, config.mqttBroker);
    _preferences.putString(PREF_MQTT_USER, config.mqttUsername);
    _preferences.putString(PREF_MQTT_PASS, config.mqttPassword);
    
    LogBox::message("Config Saved", "Configuration saved successfully");
    
    return true;
}

void ConfigManager::clearConfig() {
    if (!_initialized && !begin()) {
        return;
    }
    
    LogBox::begin("Factory Reset");
    LogBox::line("Clearing all configuration...");
    
    _preferences.clear();
    
    LogBox::end("Configuration cleared successfully");
}

// Individual getters
String ConfigManager::getWiFiSSID() {
    if (!_initialized && !begin()) return "";
    return _preferences.getString(PREF_WIFI_SSID, "");
}

String ConfigManager::getWiFiPassword() {
    if (!_initialized && !begin()) return "";
    return _preferences.getString(PREF_WIFI_PASS, "");
}

String ConfigManager::getFriendlyName() {
    if (!_initialized && !begin()) return "";
    return _preferences.getString(PREF_FRIENDLY_NAME, "");
}

String ConfigManager::getMQTTBroker() {
    if (!_initialized && !begin()) return "";
    return _preferences.getString(PREF_MQTT_BROKER, "");
}

String ConfigManager::getMQTTUsername() {
    if (!_initialized && !begin()) return "";
    return _preferences.getString(PREF_MQTT_USER, "");
}

String ConfigManager::getMQTTPassword() {
    if (!_initialized && !begin()) return "";
    return _preferences.getString(PREF_MQTT_PASS, "");
}

bool ConfigManager::getDebugMode() {
    if (!_initialized && !begin()) return false;
    return _preferences.getBool(PREF_DEBUG_MODE, false);
}

// Static IP getters
bool ConfigManager::getUseStaticIP() {
    if (!_initialized && !begin()) return false;
    return _preferences.getBool(PREF_USE_STATIC_IP, false);
}

String ConfigManager::getStaticIP() {
    if (!_initialized && !begin()) return "";
    return _preferences.getString(PREF_STATIC_IP, "");
}

String ConfigManager::getGateway() {
    if (!_initialized && !begin()) return "";
    return _preferences.getString(PREF_GATEWAY, "");
}

String ConfigManager::getSubnet() {
    if (!_initialized && !begin()) return "";
    return _preferences.getString(PREF_SUBNET, "");
}

String ConfigManager::getPrimaryDNS() {
    if (!_initialized && !begin()) return "";
    return _preferences.getString(PREF_PRIMARY_DNS, "");
}

String ConfigManager::getSecondaryDNS() {
    if (!_initialized && !begin()) return "";
    return _preferences.getString(PREF_SECONDARY_DNS, "");
}

// Individual setters
void ConfigManager::setWiFiCredentials(const String& ssid, const String& password) {
    if (!_initialized && !begin()) return;
    _preferences.putString(PREF_WIFI_SSID, ssid);
    _preferences.putString(PREF_WIFI_PASS, password);
}

void ConfigManager::setFriendlyName(const String& name) {
    if (!_initialized && !begin()) return;
    _preferences.putString(PREF_FRIENDLY_NAME, name);
}

void ConfigManager::setMQTTConfig(const String& broker, const String& username, const String& password) {
    if (!_initialized && !begin()) return;
    _preferences.putString(PREF_MQTT_BROKER, broker);
    _preferences.putString(PREF_MQTT_USER, username);
    _preferences.putString(PREF_MQTT_PASS, password);
}

void ConfigManager::setDebugMode(bool enabled) {
    if (!_initialized && !begin()) return;
    _preferences.putBool(PREF_DEBUG_MODE, enabled);
}

// WiFi channel locking
bool ConfigManager::hasWiFiChannelLock() {
    if (!_initialized && !begin()) return false;
    return _preferences.isKey(PREF_WIFI_CHANNEL);
}

uint8_t ConfigManager::getWiFiChannel() {
    if (!_initialized && !begin()) return 0;
    return _preferences.getUChar(PREF_WIFI_CHANNEL, 0);
}

void ConfigManager::getWiFiBSSID(uint8_t* bssid) {
    if (!_initialized && !begin()) return;
    _preferences.getBytes(PREF_WIFI_BSSID, bssid, 6);
}

void ConfigManager::setWiFiChannelLock(uint8_t channel, const uint8_t* bssid) {
    if (!_initialized && !begin()) return;
    _preferences.putUChar(PREF_WIFI_CHANNEL, channel);
    _preferences.putBytes(PREF_WIFI_BSSID, bssid, 6);
}

void ConfigManager::clearWiFiChannelLock() {
    if (!_initialized && !begin()) return;
    _preferences.remove(PREF_WIFI_CHANNEL);
    _preferences.remove(PREF_WIFI_BSSID);
}

bool ConfigManager::sanitizeFriendlyName(const String& input, String& output) {
    output = "";
    
    if (input.length() == 0 || input.length() > 24) {
        return false;
    }
    
    // Convert to lowercase and filter invalid characters
    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];
        
        if (c >= 'A' && c <= 'Z') {
            output += (char)(c + 32);  // Convert to lowercase
        } else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-') {
            output += c;
        }
        // Skip all other characters
    }
    
    // Remove leading/trailing hyphens
    while (output.length() > 0 && output[0] == '-') {
        output = output.substring(1);
    }
    while (output.length() > 0 && output[output.length() - 1] == '-') {
        output = output.substring(0, output.length() - 1);
    }
    
    // Check if result is valid
    if (output.length() == 0) {
        return false;
    }
    
    return true;
}

void ConfigManager::markAsConfigured() {
    if (!_initialized && !begin()) return;
    _preferences.putBool(PREF_CONFIGURED, true);
}

void ConfigManager::setConfigured(bool configured) {
    if (!_initialized && !begin()) return;
    _preferences.putBool(PREF_CONFIGURED, configured);
}

void ConfigManager::setUseStaticIP(bool enabled) {
    if (!_initialized && !begin()) return;
    _preferences.putBool(PREF_USE_STATIC_IP, enabled);
}

void ConfigManager::setStaticIPConfig(const String& ip, const String& gw, 
                                     const String& sn, const String& dns1, const String& dns2) {
    if (!_initialized && !begin()) return;
    _preferences.putString(PREF_STATIC_IP, ip);
    _preferences.putString(PREF_GATEWAY, gw);
    _preferences.putString(PREF_SUBNET, sn);
    _preferences.putString(PREF_PRIMARY_DNS, dns1);
    _preferences.putString(PREF_SECONDARY_DNS, dns2);
}

bool ConfigManager::saveConfig() {
    // This method is a no-op since individual setters already write to NVS
    // Kept for API compatibility with portal
    return true;
}
