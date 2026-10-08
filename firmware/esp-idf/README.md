# Slot-18 ESP-IDF v0.2.0 — touchscreen bring-up CANDIDATE

The source builds on Waveshare's official ESP-IDF LVGL v9 display + GT911 touch driver, adapted for a radar interface with SIMULATED values. This is NOT a real radar-reading firmware. See main/main.c and README.md at the repository root for project context.

## Computer flashing procedure
1. Install ESP-IDF 5.5.2 via VS Code Espressif IDF extension or official CLI.
2. Open this folder in IDF project mode. Set ESP32-S3 target, build with idf.py set-target esp32s3 && idf.py build.
3. Attach the board's UART-labelled USB-C programming connector to computer via data-capable USB cable, not to mains adapter while flashing. Confirm actual COM port; flash / monitor using idf.py -p COMx flash monitor (Windows) or corresponding Linux/macOS port. Follow Waveshare RESET guidance.
4. Verify the orange SIMULATED ONLY label, moving target dot and changing heading + pause buttons. Press Test to start guided steps, manually PASS/FAIL, and use UART logs to inspect diagnostics. The on-screen console is a CANDIDATE until user confirms touch works.
5. After flashing, power it with a suitable regulated USB 5V wall adapter for stable bench tests. Never wire AC mains directly into ESP32 or sensor pins.

Source of vendor drivers: https://github.com/waveshareteam/ESP32-S3-Touch-LCD-7/tree/main/examples/ESP-IDF/09_lvgl_v9_demo ; component .c is CC0-1.0. Driver header, SDK config and dependencies taken from the same official demo. Only sdkconfig flash size changed from 8MB to 16MB to match Waveshare's N16R8 configuration. No custom GPIO assumptions.

Before connecting LD2450, verify independent UART availability, 3.3V signal levels, radar VCC requirements and exact board header pinout. LD2450 wiring is a separate version milestone. Never present demo target dots as live radar or a verified human presence detector.

Diagnostics are structured over serial and a bounded flash NVS ring. Initial timestamp is monotonic uptime, not UTC. To export, monitor serial while pressing LOGS. Guided test results represent user confirmation, not automatic verification.
