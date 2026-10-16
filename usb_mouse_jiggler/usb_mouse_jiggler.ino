// usb_mouse_jiggler.ino
//
// Enumerates as a plain USB mouse and nudges the pointer a couple of pixels
// out and back every 30-40 s, so the computer never goes idle.
//
// Needs a chip with native USB-OTG (ESP32-S2/S3); built for the Seeed XIAO
// ESP32-S3. The ESP32-C3/C6 USB port is serial/JTAG only and cannot do this.
//
// Build with USB Mode = USB-OTG (TinyUSB). See usb_mouse_jiggler/README.md.

#include <Arduino.h>

#ifndef ARDUINO_USB_MODE
#error This ESP32 SoC has no native USB interface
#elif ARDUINO_USB_MODE == 1
#error USB Mode must be USB-OTG (TinyUSB), not Hardware CDC and JTAG
#endif

#include "USB.h"
#include "USBHIDMouse.h"

// ---- Tuning ----------------------------------------------------------------
static const uint32_t JIGGLE_EVERY_MS  = 30000;  // base gap between nudges
static const uint32_t JIGGLE_RANDOM_MS = 10000;  // + random 0..this, so it isn't metronomic
static const int8_t   JIGGLE_PIXELS    = 2;      // size of the nudge (it moves back afterwards)
static const uint32_t SETTLE_MS        = 40;     // pause between the out and back moves
// ----------------------------------------------------------------------------

USBHIDMouse Mouse;

void setup() {
  USB.productName("USB Mouse");
  Mouse.begin();
  USB.begin();
}

void loop() {
  delay(JIGGLE_EVERY_MS + random(JIGGLE_RANDOM_MS + 1));
  // Out and straight back, so the pointer ends up where it started.
  Mouse.move(JIGGLE_PIXELS, 0);
  delay(SETTLE_MS);
  Mouse.move(-JIGGLE_PIXELS, 0);
}
