#ifndef AP_MODE_CONTROLLER_H
#define AP_MODE_CONTROLLER_H

#include "wifi_manager.h"
#include "config_portal.h"
#include "logger.h"

/**
 * @brief Handles Access Point mode for initial device configuration
 * 
 * This controller manages the AP mode which is activated on first boot
 * when the device has no WiFi configuration. It starts an access point
 * and configuration portal to allow users to enter WiFi credentials.
 */
class APModeController {
public:
    APModeController(WiFiManager* wifi, ConfigPortal* portal);
    
    /**
     * @brief Enter AP mode and start configuration portal
     * @return true if AP mode started successfully
     */
    bool begin();
    
    /**
     * @brief Handle AP mode client requests (call in loop)
     */
    void handleClient();
    
    /**
     * @brief Check if configuration was received
     * @return true if WiFi credentials were configured
     */
    bool isConfigReceived();
    
    /**
     * @brief Get Access Point name
     */
    String getAPName();
    
    /**
     * @brief Get Access Point IP address
     */
    String getAPIP();
    
private:
    WiFiManager* wifiManager;
    ConfigPortal* configPortal;
};

#endif // AP_MODE_CONTROLLER_H
