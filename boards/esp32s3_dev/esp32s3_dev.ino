/*
   ESP32 Multi-Board Template - ESP32-S3 DevKit

   This is the ESP32-S3 DevKit specific sketch.

   Common implementation is in ../../common/src/main_sketch.ino.inc
   Board-specific configuration is in board_config.h.
*/

// Board validation
#ifndef ARDUINO_ESP32S3_DEV
#error "Wrong board selection for this sketch, please select ESP32S3 Dev Module in the boards menu."
#endif

// Include board configuration first (required by shared code)
#include "board_config.h"

// Include shared implementation
#include "main_sketch.ino.inc"
