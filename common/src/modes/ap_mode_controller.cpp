#include "ap_mode_controller.h"

APModeController::APModeController(WiFiManager* wifi, ConfigPortal* portal)
    : wifiManager(wifi), configPortal(portal) {
}

bool APModeController::begin() {
    if (wifiManager->startAccessPoint()) {
        String apName = wifiManager->getAPName();
        String apIP = wifiManager->getAPIPAddress();
        
        // Start configuration portal
        if (configPortal->begin()) {
            LogBox::begin("Configuration Portal Active");
            LogBox::line("1. Connect to WiFi: " + apName);
            LogBox::line("2. Open: http://" + apIP);
            LogBox::line("3. Enter WiFi credentials");
            LogBox::end();
            return true;
        } else {
            LogBox::message("Configuration Portal", "Failed to start configuration portal!");
            return false;
        }
    } else {
        LogBox::message("Access Point", "Failed to start Access Point!");
        return false;
    }
}

void APModeController::handleClient() {
    configPortal->handleClient();
}

bool APModeController::isConfigReceived() {
    return configPortal->isConfigReceived();
}

String APModeController::getAPName() {
    return wifiManager->getAPName();
}

String APModeController::getAPIP() {
    return wifiManager->getAPIPAddress();
}

void APModeController::runConfigurationLoop(const char* message) {
    if (message) {
        LogBox::message("Config Mode", message);
    }

    LogBox::message("AP Mode", "Connect to: " + getAPName());
    LogBox::message("AP Mode", "Configure at: http://" + getAPIP());

    // Wait for configuration
    while (!isConfigReceived()) {
        handleClient();
        delay(10);
    }

    LogBox::message("AP Mode", "Configuration received, rebooting...");
    delay(2000);
    ESP.restart();
}
