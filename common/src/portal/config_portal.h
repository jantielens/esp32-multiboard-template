#ifndef CONFIG_PORTAL_H
#define CONFIG_PORTAL_H

#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include "config_manager.h"
#include "wifi_manager.h"
#include "power_manager.h"
#include "mqtt_manager.h"
#include "ota_manager.h"

class ConfigPortal {
public:
    ConfigPortal(ConfigManager* configManager, WiFiManager* wifiManager, 
                 PowerManager* powerManager = nullptr, MQTTManager* mqttManager = nullptr);
    ~ConfigPortal();
    
    // Start the configuration web server
    bool begin(int port = 80);
    
    // Stop the configuration web server
    void stop();
    
    // Handle client requests (call in loop)
    void handleClient();
    
    // Check if configuration was submitted
    bool isConfigReceived();
    
    // Get the port number
    int getPort();
    
private:
    ConfigManager* _configManager;
    WiFiManager* _wifiManager;
    PowerManager* _powerManager;
    MQTTManager* _mqttManager;
    OTAManager _otaManager;
    WebServer* _server;
    bool _configReceived;
    int _port;

    // HTTP handlers
    void handleRoot();
    void handleSubmit();
    void handleReboot();
    void handleFactoryReset();
    void handleOTAPage();
    void handleOTAUpload();
    void handleOTAFromURL();
    void handleNotFound();

    // HTML page generators
    String generateConfigPage();
    String generateSuccessPage();
    String generateErrorPage(const String& error);
    String generateOTAPage();

    // Validation helpers
    bool validateIPv4Format(const String& ip);
};

#endif // CONFIG_PORTAL_H