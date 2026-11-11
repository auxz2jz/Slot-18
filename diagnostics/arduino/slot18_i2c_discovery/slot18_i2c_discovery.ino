/*
  Slot-18 — I2C bus discovery (hardware diagnostic candidate)
  Board: Waveshare ESP32-S3-Touch-LCD-7 (Rev1.2)
  I2C: GPIO8 SDA, GPIO9 SCL, 100 kHz
  Expected from the 2026-10-27 checkpoint:
    0x4D — CJMCU-752 SC16IS752 bridge
    0x5D — onboard touchscreen controller
    Other onboard device addresses may appear.

  This scans I2C addresses only. It does NOT:
    - Verify the C4002 UART link or radar detection;
    - Change radar settings;
    - Verify the TFT graphics/touch after flashing.
  It may leave the display blank because the sketch does not initialize LCD.
  The screen's loose red I2C VCC wire must remain DISCONNECTED/insulated;
  screen is powered by its USB-C; grounds and SDA/SCL are shared.
*/
#include <Wire.h>

static const int kSdaPin = 8;
static const int kSclPin = 9;
static bool busReady = false;

void setup() {
  Serial.begin(115200);
  delay(1800);
  Serial.println();
  Serial.println("Slot-18 I2C Bridge Discovery");
  Serial.println("Read-only address scan; no C4002 commands.");
  busReady = Wire.begin(kSdaPin, kSclPin, 100000);
  if (!busReady) {
    Serial.println("FAIL: Wire.begin(8, 9, 100000)");
  }
}

void loop() {
  if (!busReady) {
    delay(5000);
    return;
  }
  bool bridgeFound = false;
  bool touchFound = false;
  unsigned count = 0;
  Serial.println("--- Scan ---");
  for (uint8_t address = 0x08; address <= 0x77; ++address) {
    Wire.beginTransmission(address);
    const uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("ACK 0x%02X", address);
      if (address == 0x4D) {
        bridgeFound = true;
        Serial.print(" (CJMCU-752 expected)");
      }
      if (address == 0x5D) {
        touchFound = true;
        Serial.print(" (touch controller expected)");
      }
      Serial.println();
      ++count;
    }
  }
  Serial.printf("Found %u responding I2C addresses.\n", count);
  Serial.printf("CJMCU-752 0x4D: %s\n", bridgeFound ? "PASS (I2C ACK only)" : "NOT FOUND");
  Serial.printf("Touch 0x5D: %s\n", touchFound ? "PASS (I2C ACK only)" : "NOT FOUND");
  Serial.println("C4002 UART not tested by this scan.");
  delay(5000);
}
