#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <Update.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClient.h>

/**
 * @brief OTA Update Manager
 * 
 * Handles firmware updates from HTTP URLs or binary uploads.
 * Supports progress tracking and error reporting.
 */
class OTAManager {
public:
    /**
     * @brief Progress callback function type
     * @param current Current bytes downloaded/written
     * @param total Total bytes to download/write
     */
    typedef void (*ProgressCallback)(size_t current, size_t total);
    
    /**
     * @brief Update status information
     */
    struct UpdateStatus {
        bool inProgress;
        bool success;
        size_t bytesWritten;
        size_t totalBytes;
        int percentComplete;
        String errorMessage;
        
        UpdateStatus() : inProgress(false), success(false), 
                        bytesWritten(0), totalBytes(0), 
                        percentComplete(0) {}
    };
    
    OTAManager();
    ~OTAManager();
    
    /**
     * @brief Download and install firmware from HTTP URL
     * @param firmwareUrl Direct URL to .bin firmware file
     * @param progressCallback Optional callback for progress updates
     * @return true if download and installation succeeded
     */
    bool updateFromURL(const String& firmwareUrl, ProgressCallback progressCallback = nullptr);
    
    /**
     * @brief Begin binary upload (call before writing chunks)
     * @param expectedSize Expected firmware size in bytes (0 for unknown)
     * @return true if update partition is ready
     */
    bool beginUpload(size_t expectedSize = 0);
    
    /**
     * @brief Write firmware chunk during upload
     * @param data Firmware data buffer
     * @param len Length of data in buffer
     * @return Number of bytes written (0 on error)
     */
    size_t writeUpload(uint8_t* data, size_t len);
    
    /**
     * @brief Finalize upload and verify firmware
     * @return true if firmware is valid and ready to boot
     */
    bool endUpload();
    
    /**
     * @brief Abort ongoing upload
     */
    void abortUpload();
    
    /**
     * @brief Get current update status
     * @return Status structure with progress info
     */
    UpdateStatus getStatus() const { return _status; }
    
    /**
     * @brief Get last error message
     * @return Error description string
     */
    String getLastError() const { return _status.errorMessage; }
    
    /**
     * @brief Check if update is in progress
     * @return true if currently updating
     */
    bool isInProgress() const { return _status.inProgress; }

private:
    UpdateStatus _status;
    HTTPClient _http;
    
    /**
     * @brief Update progress tracking
     * @param current Current bytes
     * @param total Total bytes
     */
    void updateProgress(size_t current, size_t total);
};

#endif // OTA_MANAGER_H
