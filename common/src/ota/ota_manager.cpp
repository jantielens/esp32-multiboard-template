#include "ota_manager.h"
#include "logger.h"

OTAManager::OTAManager() {
}

OTAManager::~OTAManager() {
    _http.end();
}

bool OTAManager::updateFromURL(const String& firmwareUrl, ProgressCallback progressCallback) {
    _status = UpdateStatus();
    _status.inProgress = true;
    
    LogBox::begin("HTTP OTA Update");
    LogBox::line("Downloading firmware...");
    LogBox::line("URL: " + firmwareUrl);
    LogBox::end();
    
    // Configure HTTP client
    _http.begin(firmwareUrl);
    _http.setTimeout(300000); // 5 minute timeout for download
    _http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    // Start HTTP request
    int httpCode = _http.GET();
    
    if (httpCode != HTTP_CODE_OK) {
        _status.errorMessage = "Download failed: HTTP " + String(httpCode);
        LogBox::message("OTA Error", _status.errorMessage);
        _http.end();
        _status.inProgress = false;
        return false;
    }
    
    // Get content length
    int contentLength = _http.getSize();
    
    if (contentLength <= 0) {
        _status.errorMessage = "Invalid content length";
        LogBox::message("OTA Error", _status.errorMessage);
        _http.end();
        _status.inProgress = false;
        return false;
    }
    
    _status.totalBytes = contentLength;
    
    LogBox::messagef("HTTP OTA", "Firmware size: %d KB", contentLength / 1024);
    
    // Begin OTA update
    if (!Update.begin(contentLength)) {
        _status.errorMessage = "Not enough space for OTA update";
        LogBox::message("OTA Error", _status.errorMessage);
        _http.end();
        _status.inProgress = false;
        return false;
    }
    
    // Get stream
    WiFiClient* stream = _http.getStreamPtr();
    
    // Download and write firmware in chunks
    size_t written = 0;
    uint8_t buffer[4096];
    
    LogBox::message("HTTP OTA", "Writing firmware...");
    
    while (_http.connected() && (written < contentLength)) {
        // Get available data size
        size_t available = stream->available();
        
        if (available) {
            // Read up to buffer size
            size_t toRead = min(available, sizeof(buffer));
            size_t bytesRead = stream->readBytes(buffer, toRead);
            
            // Write to update partition
            size_t bytesWritten = Update.write(buffer, bytesRead);
            
            if (bytesWritten != bytesRead) {
                _status.errorMessage = "Write error during OTA update";
                LogBox::message("OTA Error", _status.errorMessage);
                Update.abort();
                _http.end();
                _status.inProgress = false;
                return false;
            }
            
            written += bytesWritten;
            
            // Update progress
            updateProgress(written, contentLength);
            
            // Call progress callback if provided
            if (progressCallback) {
                progressCallback(written, contentLength);
            }
            
            // Log progress every 100KB
            static size_t lastLoggedKB = 0;
            size_t currentKB = written / 1024;
            if (currentKB - lastLoggedKB >= 100) {
                LogBox::messagef("OTA Progress", "%d KB / %d KB (%d%%)", 
                    currentKB, 
                    contentLength / 1024, 
                    (written * 100) / contentLength);
                lastLoggedKB = currentKB;
            }
        }
        
        delay(1);
    }
    
    _http.end();
    
    // Verify download completed
    if (written != contentLength) {
        _status.errorMessage = "Download incomplete: " + String(written) + " / " + String(contentLength);
        LogBox::message("OTA Error", _status.errorMessage);
        Update.abort();
        _status.inProgress = false;
        return false;
    }
    
    // Finalize update
    if (!Update.end(true)) {
        _status.errorMessage = "Update finalization failed: " + String(Update.getError());
        LogBox::message("OTA Error", _status.errorMessage);
        _status.inProgress = false;
        return false;
    }
    
    LogBox::begin("HTTP OTA");
    LogBox::line("✓ Firmware update successful!");
    LogBox::line("Device will reboot...");
    LogBox::end();
    
    _status.inProgress = false;
    _status.success = true;
    _status.percentComplete = 100;
    
    return true;
}

bool OTAManager::beginUpload(size_t expectedSize) {
    _status = UpdateStatus();
    _status.inProgress = true;
    
    // Calculate available OTA partition size
    size_t updateSize = expectedSize;
    if (updateSize == 0) {
        updateSize = UPDATE_SIZE_UNKNOWN;
    }
    
    LogBox::begin("File Upload OTA");
    LogBox::linef("Starting update (size: %d bytes)", expectedSize);
    LogBox::end();
    
    // Disable watchdog timer to prevent reboot during update
    disableCore0WDT();
    
    // Begin update
    if (!Update.begin(updateSize, U_FLASH)) {
        _status.errorMessage = "Failed to begin update: " + String(Update.getError());
        Update.printError(Serial);
        enableCore0WDT();
        _status.inProgress = false;
        return false;
    }
    
    _status.totalBytes = expectedSize;
    return true;
}

size_t OTAManager::writeUpload(uint8_t* data, size_t len) {
    if (!_status.inProgress) {
        return 0;
    }
    
    size_t written = Update.write(data, len);
    
    if (written != len) {
        _status.errorMessage = "Write error: " + String(Update.getError());
        Update.printError(Serial);
        return 0;
    }
    
    _status.bytesWritten += written;
    updateProgress(_status.bytesWritten, _status.totalBytes);
    
    // Log progress every 100KB
    static size_t lastLoggedKB = 0;
    size_t currentKB = _status.bytesWritten / 1024;
    if (currentKB - lastLoggedKB >= 100) {
        LogBox::messagef("Upload Progress", "%d KB / %d KB (%d%%)", 
            currentKB, 
            _status.totalBytes / 1024, 
            _status.percentComplete);
        lastLoggedKB = currentKB;
    }
    
    return written;
}

bool OTAManager::endUpload() {
    if (!_status.inProgress) {
        return false;
    }
    
    bool success = Update.end(true);
    
    if (success) {
        LogBox::begin("Upload OTA");
        LogBox::linef("✓ Update successful: %u bytes", _status.bytesWritten);
        LogBox::end();
        _status.success = true;
        _status.percentComplete = 100;
    } else {
        _status.errorMessage = "Update finalization failed: " + String(Update.getError());
        Update.printError(Serial);
        enableCore0WDT();
    }
    
    _status.inProgress = false;
    return success;
}

void OTAManager::abortUpload() {
    if (_status.inProgress) {
        Update.end();
        enableCore0WDT();
        _status.inProgress = false;
        _status.errorMessage = "Upload aborted by user";
        LogBox::message("Upload OTA", "Update aborted");
    }
}

void OTAManager::updateProgress(size_t current, size_t total) {
    _status.bytesWritten = current;
    _status.totalBytes = total;
    
    if (total > 0) {
        _status.percentComplete = (current * 100) / total;
    }
}
