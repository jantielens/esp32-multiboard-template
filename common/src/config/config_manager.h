#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    // Initialize the configuration manager
    bool begin();
    
    // Check if device has been configured
    bool isConfigured();
    
    // Check if WiFi credentials are configured
    bool hasWiFiConfig();
    
    // Load configuration from storage
    bool loadConfig(DeviceConfig& config);
    
    // Save configuration to storage
    bool saveConfig(const DeviceConfig& config);
    
    // Clear all configuration (factory reset)
    void clearConfig();
    
    // Individual getters
    String getWiFiSSID();
    String getWiFiPassword();
    String getFriendlyName();
    String getMQTTBroker();
    String getMQTTUsername();
    String getMQTTPassword();
    bool getDebugMode();
    
    // Static IP getters
    bool getUseStaticIP();
    String getStaticIP();
    String getGateway();
    String getSubnet();
    String getPrimaryDNS();
    String getSecondaryDNS();
    
    // Individual setters
    void setWiFiCredentials(const String& ssid, const String& password);
    void setFriendlyName(const String& name);
    void setMQTTConfig(const String& broker, const String& username, const String& password);
    void setDebugMode(bool enabled);
    void setConfigured(bool configured);  // Mark device as configured
    
    // Static IP setters
    void setUseStaticIP(bool enabled);
    void setStaticIPConfig(const String& ip, const String& gw, 
                          const String& sn, const String& dns1, const String& dns2);
    
    // Simplified save method (saves current state)
    bool saveConfig();
    
    // WiFi channel locking (for fast reconnection)
    bool hasWiFiChannelLock();
    uint8_t getWiFiChannel();
    void getWiFiBSSID(uint8_t* bssid);  // Copies 6 bytes to provided array
    void setWiFiChannelLock(uint8_t channel, const uint8_t* bssid);
    void clearWiFiChannelLock();
    
    // Friendly name validation and sanitization
    // Returns true if sanitization successful, false if input completely invalid
    // Rules: lowercase a-z, digits 0-9, hyphens; max 24 chars; no leading/trailing hyphens
    static bool sanitizeFriendlyName(const String& input, String& output);
    
    // Mark device as configured
    void markAsConfigured();
    
private:
    Preferences _preferences;
    bool _initialized;
};

#endif // CONFIG_MANAGER_H
