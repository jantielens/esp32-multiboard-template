/*
   ESP32 Multi-Board Template - ESP32 DevKit V1

   This is the ESP32 DevKit V1 specific sketch.

   Common implementation is in ../../common/src/main_sketch.ino.inc
   Board-specific configuration is in board_config.h.
*/

// Board validation
#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_LOLIN_D32)
#error "Wrong board selection for this sketch, please select ESP32 Dev Module or similar ESP32 board in the boards menu."
#endif

// Include board configuration first (required by shared code)
#include "board_config.h"

// Include shared implementation
#include <src/main_sketch.ino.inc>
