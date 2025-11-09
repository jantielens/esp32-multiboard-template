#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================
// STORAGE NAMESPACE (customize this!)
// ============================================
#define PREF_NAMESPACE "esp32cfg"  // TEMPLATE: Change to your project name

// ============================================
// CORE WIFI SETTINGS (always needed)
// ============================================
#define PREF_CONFIGURED "configured"
#define PREF_WIFI_SSID "wifi_ssid"
#define PREF_WIFI_PASS "wifi_pass"
#define PREF_USE_STATIC_IP "use_static_ip"
#define PREF_STATIC_IP "static_ip"
#define PREF_GATEWAY "gateway"
#define PREF_SUBNET "subnet"
#define PREF_PRIMARY_DNS "dns1"
#define PREF_SECONDARY_DNS "dns2"

// WiFi channel locking (for fast reconnection)
#define PREF_WIFI_CHANNEL "wifi_ch"
#define PREF_WIFI_BSSID "wifi_bssid"

// ============================================
// OPTIONAL COMMON SETTINGS
// ============================================
#define PREF_FRIENDLY_NAME "friendly_name"
#define PREF_DEBUG_MODE "debug_mode"

// ============================================
// MQTT SETTINGS (core template feature)
// ============================================
#define PREF_MQTT_BROKER "mqtt_broker"
#define PREF_MQTT_USER "mqtt_user"
#define PREF_MQTT_PASS "mqtt_pass"

// ============================================
// CUSTOM SETTINGS (add yours here)
// ============================================
// #define PREF_MY_SETTING "my_setting"

// ============================================
// DEVICE CONFIGURATION STRUCTURE
// ============================================
struct DeviceConfig {
    // WiFi credentials
    String wifiSSID;
    String wifiPassword;
    
    // Static IP configuration
    bool useStaticIP;
    String staticIP;
    String gateway;
    String subnet;
    String primaryDNS;
    String secondaryDNS;
    
    // Device info
    String friendlyName;
    bool isConfigured;
    bool debugMode;
    
    // MQTT settings (core template feature - optional to use)
    String mqttBroker;    // e.g., "mqtt://192.168.1.100:1883"
    String mqttUsername;  // Optional
    String mqttPassword;  // Optional
    
    // TEMPLATE: Add your custom fields here
    // Example:
    // int refreshInterval;
    // String customApiKey;
    
    // Constructor with defaults
    DeviceConfig() : 
        wifiSSID(""),
        wifiPassword(""),
        useStaticIP(false),
        staticIP(""),
        gateway(""),
        subnet(""),
        primaryDNS(""),
        secondaryDNS(""),
        friendlyName(""),
        isConfigured(false),
        debugMode(false),
        mqttBroker(""),
        mqttUsername(""),
        mqttPassword("")
        // TEMPLATE: Initialize your custom fields
    {}
};

#endif // CONFIG_H
