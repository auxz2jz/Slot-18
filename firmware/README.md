# Target firmware (not started)
Hardware target: Waveshare ESP32-S3-Touch-LCD-7 (capacitive touch, ESP32-S3-WROOM-1-N16R8).
**This directory does not contain a buildable flash image or ESP-IDF project yet.**

After hardware arrives:
1. Read master instructions + PROJECT_MEMORY + ROADMAP + diagnostics doc.
2. Download a known-current matching Waveshare ESP-IDF LVGL demonstration and pinout; confirm board model, touch variant and memory.
3. Build and flash vendor sample unchanged. Record compile version, logs and user-observed screen/touch result.
4. Port the platform-neutral data contract and radar UI concept from prototype/ to LVGL, then begin serial sensors individually.
5. Store baseline only after user physically confirms it; do not invent ESP32 build success.

Docs: https://docs.waveshare.com/ESP32-S3-Touch-LCD-7
