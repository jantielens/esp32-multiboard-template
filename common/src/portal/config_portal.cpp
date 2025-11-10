#include "config_portal.h"
#include "config_portal_html.h"
#include "config_portal_css.h"
#include "logger.h"
#include "package_config.h"
#include "board_config.h"

// Helper function to generate HTML header with dynamic title
static String getHtmlHeader() {
    String header = String(HTML_PAGE_HEADER);
    header.replace("{{TITLE}}", String(PACKAGE_DISPLAY_NAME) + " Configuration");
    return header;
}

ConfigPortal::ConfigPortal(ConfigManager* configManager, WiFiManager* wifiManager,
                           PowerManager* powerManager, MQTTManager* mqttManager)
    : _configManager(configManager), _wifiManager(wifiManager),
      _powerManager(powerManager), _mqttManager(mqttManager),
      _server(nullptr), _configReceived(false), _port(80) {
}

ConfigPortal::~ConfigPortal() {
    stop();
}

bool ConfigPortal::begin(int port) {
    if (_server != nullptr) {
        LogBox::message("Config Portal", "Already running");
        return true;
    }
    
    _port = port;
    _server = new WebServer(_port);
    
    // Set up routes
    _server->on("/", [this]() { this->handleRoot(); });
    _server->on("/submit", HTTP_POST, [this]() { this->handleSubmit(); });
    _server->on("/reboot", HTTP_POST, [this]() { this->handleReboot(); });
    _server->on("/factory-reset", HTTP_POST, [this]() { this->handleFactoryReset(); });
    
    // OTA routes
    _server->on("/ota", HTTP_GET, [this]() { this->handleOTAPage(); });
    _server->on("/ota/upload", HTTP_POST,
        [this]() { this->handleOTAUpload(); },
        [this]() {
            // Handle file upload chunks
            HTTPUpload& upload = _server->upload();
            
            if (upload.status == UPLOAD_FILE_START) {
                LogBox::message("OTA Upload", "Starting: " + upload.filename);
                _otaManager.beginUpload(upload.totalSize);
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                _otaManager.writeUpload(upload.buf, upload.currentSize);
            } else if (upload.status == UPLOAD_FILE_END) {
                LogBox::messagef("OTA Upload", "Completed: %u bytes", upload.totalSize);
            } else if (upload.status == UPLOAD_FILE_ABORTED) {
                _otaManager.abortUpload();
            }
        }
    );
    _server->on("/ota/url", HTTP_POST, [this]() { this->handleOTAFromURL(); });
    
    _server->onNotFound([this]() { this->handleNotFound(); });
    
    _server->begin();
    
    LogBox::begin("Config Portal");
    LogBox::line("Portal started on port " + String(_port));
    LogBox::line("Access at: http://" + _wifiManager->getAPIPAddress());
    LogBox::end();
    
    return true;
}

void ConfigPortal::stop() {
    if (_server != nullptr) {
        _server->stop();
        delete _server;
        _server = nullptr;
        LogBox::message("Config Portal", "Stopped");
    }
}

void ConfigPortal::handleClient() {
    if (_server != nullptr) {
        _server->handleClient();
    }
}

bool ConfigPortal::isConfigReceived() {
    return _configReceived;
}

int ConfigPortal::getPort() {
    return _port;
}

void ConfigPortal::handleRoot() {
    _server->send(200, "text/html", generateConfigPage());
}

void ConfigPortal::handleSubmit() {
    LogBox::begin("Config Submission");
    
    // Get WiFi credentials
    String ssid = _server->arg("ssid");
    String password = _server->arg("password");
    
    if (ssid.length() == 0) {
        LogBox::line("ERROR: WiFi SSID is required");
        LogBox::end();
        _server->send(400, "text/html", generateErrorPage("WiFi SSID is required"));
        return;
    }
    
    // Get friendly name (optional)
    String friendlyName = _server->arg("friendlyName");
    
    // Get static IP settings
    bool useStaticIP = _server->hasArg("useStaticIP");
    String staticIP = _server->arg("staticIP");
    String gateway = _server->arg("gateway");
    String subnet = _server->arg("subnet");
    String primaryDNS = _server->arg("primaryDNS");
    String secondaryDNS = _server->arg("secondaryDNS");
    
    // Validate static IP settings if enabled
    if (useStaticIP) {
        if (!validateIPv4Format(staticIP) || !validateIPv4Format(gateway) || 
            !validateIPv4Format(subnet) || !validateIPv4Format(primaryDNS)) {
            LogBox::line("ERROR: Invalid static IP configuration");
            LogBox::end();
            _server->send(400, "text/html", generateErrorPage("Invalid static IP configuration"));
            return;
        }
    }
    
    // Get MQTT settings (optional)
    String mqttBroker = _server->arg("mqttBroker");
    String mqttUsername = _server->arg("mqttUsername");
    String mqttPassword = _server->arg("mqttPassword");
    
    // Get debug mode
    bool debugMode = _server->hasArg("debugMode");
    
    // Save WiFi credentials
    _configManager->setWiFiCredentials(ssid, password);
    LogBox::line("WiFi SSID: " + ssid);
    
    // Save friendly name if provided
    if (friendlyName.length() > 0) {
        _configManager->setFriendlyName(friendlyName);
        LogBox::line("Friendly name: " + friendlyName);
    }
    
    // Save static IP settings
    _configManager->setUseStaticIP(useStaticIP);
    if (useStaticIP) {
        _configManager->setStaticIPConfig(staticIP, gateway, subnet, primaryDNS, secondaryDNS);
        LogBox::line("Static IP: " + staticIP);
    }
    
    // Save MQTT settings if provided
    if (mqttBroker.length() > 0) {
        _configManager->setMQTTConfig(mqttBroker, mqttUsername, mqttPassword);
        LogBox::line("MQTT Broker: " + mqttBroker);
    }
    
    // Save debug mode
    _configManager->setDebugMode(debugMode);
    
    // Mark as configured
    _configManager->setConfigured(true);
    
    // Save configuration
    if (!_configManager->saveConfig()) {
        LogBox::line("ERROR: Failed to save configuration");
        LogBox::end();
        _server->send(500, "text/html", generateErrorPage("Failed to save configuration"));
        return;
    }
    
    LogBox::line("Configuration saved successfully");
    LogBox::end();
    
    _configReceived = true;
    _server->send(200, "text/html", generateSuccessPage());
}

void ConfigPortal::handleReboot() {
    _server->send(200, "text/html", "Rebooting...");
    delay(1000);
    ESP.restart();
}

void ConfigPortal::handleNotFound() {
    _server->send(404, "text/plain", "Not Found");
}

String ConfigPortal::generateConfigPage() {
    String deviceID = _wifiManager->getDeviceIdentifier();
    String currentSSID = _configManager->getWiFiSSID();
    String currentPassword = _configManager->getWiFiPassword();
    String currentFriendlyName = _configManager->getFriendlyName();
    bool useStaticIP = _configManager->getUseStaticIP();
    String staticIP = _configManager->getStaticIP();
    String gateway = _configManager->getGateway();
    String subnet = _configManager->getSubnet();
    String primaryDNS = _configManager->getPrimaryDNS();
    String secondaryDNS = _configManager->getSecondaryDNS();
    String mqttBroker = _configManager->getMQTTBroker();
    String mqttUsername = _configManager->getMQTTUsername();
    String mqttPassword = _configManager->getMQTTPassword();
    bool debugMode = _configManager->getDebugMode();
    
    String html = getHtmlHeader();
    html += "<style>" + String(CONFIG_PORTAL_CSS) + "</style>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>" + String(PACKAGE_DISPLAY_NAME) + " Configuration</h1>";
    html += "<p class='device-id'>Device: " + deviceID + " (" + String(BOARD_NAME) + ")</p>";
    html += "<form method='POST' action='/submit'>";
    
    // WiFi settings
    html += "<div class='section'>";
    html += "<h2>WiFi Settings</h2>";
    html += "<label>WiFi SSID*</label>";
    html += "<input type='text' name='ssid' value='" + currentSSID + "' required>";
    html += "<label>WiFi Password</label>";
    html += "<input type='password' name='password' value='" + currentPassword + "'>";
    html += "</div>";
    
    // Device settings
    html += "<div class='section'>";
    html += "<h2>Device Settings</h2>";
    html += "<label>Friendly Name</label>";
    html += "<input type='text' name='friendlyName' value='" + currentFriendlyName + "' placeholder='my-device'>";
    html += "</div>";
    
    // Static IP settings
    html += "<div class='section'>";
    html += "<h2>Network Settings</h2>";
    html += "<label><input type='checkbox' name='useStaticIP' " + String(useStaticIP ? "checked" : "") + " onchange='toggleStaticIP(this)'> Use Static IP</label>";
    html += "<div id='staticIPFields' style='display:" + String(useStaticIP ? "block" : "none") + "'>";
    html += "<label>Static IP</label>";
    html += "<input type='text' name='staticIP' value='" + staticIP + "' placeholder='192.168.1.100'>";
    html += "<label>Gateway</label>";
    html += "<input type='text' name='gateway' value='" + gateway + "' placeholder='192.168.1.1'>";
    html += "<label>Subnet Mask</label>";
    html += "<input type='text' name='subnet' value='" + subnet + "' placeholder='255.255.255.0'>";
    html += "<label>Primary DNS</label>";
    html += "<input type='text' name='primaryDNS' value='" + primaryDNS + "' placeholder='8.8.8.8'>";
    html += "<label>Secondary DNS</label>";
    html += "<input type='text' name='secondaryDNS' value='" + secondaryDNS + "' placeholder='8.8.4.4'>";
    html += "</div>";
    html += "</div>";
    
    // MQTT settings
    html += "<div class='section'>";
    html += "<h2>MQTT Settings (Optional)</h2>";
    html += "<label>MQTT Broker</label>";
    html += "<input type='text' name='mqttBroker' value='" + mqttBroker + "' placeholder='mqtt://192.168.1.10:1883'>";
    html += "<label>MQTT Username</label>";
    html += "<input type='text' name='mqttUsername' value='" + mqttUsername + "'>";
    html += "<label>MQTT Password</label>";
    html += "<input type='password' name='mqttPassword' value='" + mqttPassword + "'>";
    html += "</div>";
    
    // Advanced settings
    html += "<div class='section'>";
    html += "<h2>Advanced</h2>";
    html += "<label><input type='checkbox' name='debugMode' " + String(debugMode ? "checked" : "") + "> Enable Debug Logging</label>";
    html += "</div>";

    html += "<button type='submit' class='btn-primary'>Save Configuration</button>";
    html += "</form>";
    
    // Add utility links
    html += "<div class='section'>";
    html += "<h2>Device Management</h2>";
    html += "<a href='/ota' class='btn-secondary' style='display:inline-block;margin:10px 10px 10px 0;text-decoration:none;text-align:center;'>Firmware Update (OTA)</a>";
    html += "<form method='POST' action='/factory-reset' style='display:inline-block;margin:10px 0;' onsubmit='return confirm(\"This will erase all settings. Are you sure?\")'>";
    html += "<button type='submit' class='btn-danger'>Factory Reset</button>";
    html += "</form>";
    html += "</div>";
    
    html += "</div>";

    // JavaScript
    html += "<script>";
    html += "function toggleStaticIP(cb) {";
    html += "  document.getElementById('staticIPFields').style.display = cb.checked ? 'block' : 'none';";
    html += "}";
    html += "</script>";
    
    html += "</body></html>";
    return html;
}

String ConfigPortal::generateSuccessPage() {
    String html = getHtmlHeader();
    html += "<style>" + String(CONFIG_PORTAL_CSS) + "</style>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Configuration Saved</h1>";
    html += "<p>Your configuration has been saved successfully.</p>";
    html += "<p>The device will automatically reboot and connect to your WiFi network.</p>";
    html += "<p>You can close this page.</p>";
    html += "</div></body></html>";
    return html;
}

String ConfigPortal::generateOTAPage() {
    String html = getHtmlHeader();
    html += "<style>" + String(CONFIG_PORTAL_CSS) + "</style>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Firmware Update (OTA)</h1>";
    
    html += "<div class='card'>";
    html += "<h2>Upload Firmware File</h2>";
    html += "<form method='POST' action='/ota/upload' enctype='multipart/form-data'>";
    html += "<div class='form-group'>";
    html += "<label>Firmware Binary (.bin file):</label>";
    html += "<input type='file' name='firmware' accept='.bin' required>";
    html += "</div>";
    html += "<button type='submit' class='btn-primary'>Upload and Install</button>";
    html += "</form>";
    html += "</div>";
    
    html += "<div class='card'>";
    html += "<h2>Update from URL</h2>";
    html += "<form method='POST' action='/ota/url'>";
    html += "<div class='form-group'>";
    html += "<label>Firmware URL:</label>";
    html += "<input type='url' name='firmwareUrl' placeholder='http://example.com/firmware.bin' required>";
    html += "<small>Direct HTTP/HTTPS URL to .bin file</small>";
    html += "</div>";
    html += "<button type='submit' class='btn-primary'>Download and Install</button>";
    html += "</form>";
    html += "</div>";
    
    html += "<div class='card'>";
    html += "<a href='/' class='btn-secondary'>Back to Configuration</a>";
    html += "</div>";
    
    html += "</div></body></html>";
    return html;
}

String ConfigPortal::generateErrorPage(const String& error) {
    String html = getHtmlHeader();
    html += "<style>" + String(CONFIG_PORTAL_CSS) + "</style>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Error</h1>";
    html += "<p class='error'>" + error + "</p>";
    html += "<a href='/'>Go Back</a>";
    html += "</div></body></html>";
    return html;
}

void ConfigPortal::handleFactoryReset() {
    LogBox::message("Factory Reset", "Resetting device to factory defaults");

    _configManager->clearConfig();

    String html = getHtmlHeader();
    html += "<style>" + String(CONFIG_PORTAL_CSS) + "</style>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Factory Reset Complete</h1>";
    html += "<p>Device has been reset to factory defaults.</p>";
    html += "<p>The device will reboot now.</p>";
    html += "</div>";
    html += "<script>setTimeout(function(){window.location.href='/reboot';}, 2000);</script>";
    html += "</body></html>";

    _server->send(200, "text/html", html);
    delay(1000);
}

void ConfigPortal::handleOTAPage() {
    _server->send(200, "text/html", generateOTAPage());
}

void ConfigPortal::handleOTAUpload() {
    // This is called after the upload handler finishes
    if (Update.hasError()) {
        String errorMsg = "Upload failed. Error: " + _otaManager.getLastError();
        LogBox::message("OTA Error", errorMsg);
        
        String html = getHtmlHeader();
        html += "<style>" + String(CONFIG_PORTAL_CSS) + "</style>";
        html += "</head><body>";
        html += "<div class='container'>";
        html += "<h1>Update Failed</h1>";
        html += "<p class='error'>" + errorMsg + "</p>";
        html += "<a href='/ota'>Try Again</a>";
        html += "</div></body></html>";
        
        _server->send(500, "text/html", html);
    } else {
        if (!_otaManager.endUpload()) {
            String errorMsg = "Finalization failed: " + _otaManager.getLastError();
            LogBox::message("OTA Error", errorMsg);
            
            String html = getHtmlHeader();
            html += "<style>" + String(CONFIG_PORTAL_CSS) + "</style>";
            html += "</head><body>";
            html += "<div class='container'>";
            html += "<h1>Update Failed</h1>";
            html += "<p class='error'>" + errorMsg + "</p>";
            html += "<a href='/ota'>Try Again</a>";
            html += "</div></body></html>";
            
            _server->send(500, "text/html", html);
            return;
        }
        
        LogBox::message("OTA Success", "Firmware uploaded successfully");
        
        String html = getHtmlHeader();
        html += "<style>" + String(CONFIG_PORTAL_CSS) + "</style>";
        html += "</head><body>";
        html += "<div class='container'>";
        html += "<h1>Update Successful!</h1>";
        html += "<p>Firmware has been uploaded successfully.</p>";
        html += "<p>Device will reboot in 3 seconds...</p>";
        html += "</div>";
        html += "<script>setTimeout(function(){ESP.restart();}, 3000);</script>";
        html += "</body></html>";
        
        _server->send(200, "text/html", html);
        delay(3000);
        ESP.restart();
    }
}

void ConfigPortal::handleOTAFromURL() {
    String firmwareUrl = _server->arg("firmwareUrl");
    
    if (firmwareUrl.length() == 0) {
        _server->send(400, "text/html", generateErrorPage("Firmware URL is required"));
        return;
    }
    
    LogBox::begin("OTA from URL");
    LogBox::line("URL: " + firmwareUrl);
    LogBox::end();
    
    // Send immediate response
    String html = getHtmlHeader();
    html += "<style>" + String(CONFIG_PORTAL_CSS) + "</style>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Downloading Firmware...</h1>";
    html += "<p>Downloading from: " + firmwareUrl + "</p>";
    html += "<p>This may take several minutes. Do not power off the device.</p>";
    html += "</div></body></html>";
    
    _server->send(200, "text/html", html);
    _server->handleClient(); // Ensure response is sent
    delay(100);
    
    // Perform the update
    if (_otaManager.updateFromURL(firmwareUrl)) {
        LogBox::message("OTA Success", "Update complete, rebooting...");
        delay(2000);
        ESP.restart();
    } else {
        LogBox::message("OTA Error", _otaManager.getLastError());
        // Device will remain running, user can try again via portal
    }
}

bool ConfigPortal::validateIPv4Format(const String& ip) {
    if (ip.length() == 0) return false;
    
    IPAddress addr;
    return addr.fromString(ip);
}
