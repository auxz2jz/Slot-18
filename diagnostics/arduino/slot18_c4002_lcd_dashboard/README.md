## Known runtime crash and v2 fix (2026-11-10)

The user's FIRST LCD-dashboard upload compiled successfully but **LCD was completely blank**. User uploaded ~6,553 Serial Monitor lines; >250 cycles immediately printed:
```
E (152) i2c: CONFLICT! driver_ng is not allowed to be used with this old driver
abort() was called ...
Rebooting...
```
No `SLOT-18` splash/debug output was emitted: abort before Arduino `setup()`. A separate core dump CRC warning appears *after* abort and is likely secondary, not proof of broken flash or damaged hardware.

**Cause:** installed `ESP32_Display_Panel` / CH422G I/O-expander drives legacy ESP-IDF I2C, while our original sketch imported Arduino `Wire.h`, which on recent Arduino-ESP32 core imports the *new* I2C driver. The two are incompatible in one binary. This precisely matches [Espressif library issue #206](https://github.com/esp-arduino-libs/ESP32_Display_Panel/issues/206).

**Fix v2 (candidate, uncompiled and untested after fix):** the `slot18_c4002_lcd_dashboard.ino` sketch was modified at commit `6249e0869c800a0b05203d267ea0bf9a7ad9f3fb`: remove the Arduino `Wire` import/usage and instead access the radar bridge via `driver/i2c.h` legacy `i2c_master_write_to_device` and `i2c_master_write_read_device`, **sharing** the existing legacy I2C0 bus configured by `board->begin()` for mandatory CH422G expander on GPIO8/9. No new host driver install, no changes to proven radar sensor settings, wiring, or other sketches. The manufacturer uses the expander for display controls; do not disable it.

**First bounded next step for user:** Get updated `.ino` from this folder (not the old local copy). Replace only `slot18_c4002_lcd_dashboard.ino` in the existing Arduino sketch folder, leaving `esp_panel_board_custom_conf.h`, `esp_lv_adapter_arduino.h` and `.cpp` untouched. Close/reopen changed sketch in Arduino IDE, click **Verify only**. Send FIRST compile error or say pass. After pass, upload and check Waveshare LCD + Serial for `SLOT-18 ... v2`. If boot still conflicts, collect first 20 serial lines and library versions. Do not remove `Wire` from any previously user-verified standalone radar sketches; they do not import LCD legacy-driver library.

---
# Slot-18 — Waveshare 7-inch live radar dashboard (candidate v1)

**Status:** NEW / source-assembled / NOT Arduino-compiled or user-hardware tested. Preserve proven sketches elsewhere under `diagnostics/arduino/`.

## What should appear

A dark full-screen 800×480 dashboard **on the Waveshare's own LCD** with:
- **Presence distance** in meters and its 0–11m bar
- **Motion distance** in meters and its 0–11m bar
- Radar state: MOTION, PRESENCE, NO TARGET, or NO NEW DATA
- Link status (LIVE if checksum-valid radar notification within 5 seconds)
- Valid/corrupt packet and I2C error counts

It uses the same proven parser as `slot18_c4002_live_plotter.ino` and makes **no radar-setting writes**. The target of the milestone is to show the existing live readings *directly* on the Waveshare LCD, independent of the PC Serial Plotter.

This v1 **intentionally leaves touch disabled** (the ESP_OPEN_TOUCH macro in local board config) to avoid additional I2C bus traffic while proving LCD + CJMCU-752 on the same I2C pins. After verifying live readout, we'll add touchscreen interaction as a separate task.

## Dependencies

This sketch follows Waveshare's current 2026 [official 10_lvgl_v9_demo](https://github.com/waveshareteam/ESP32-S3-Touch-LCD-7/tree/main/examples/Arduino/examples/10_lvgl_v9_demo) setup:
- Board package: ESP32 by Espressif Systems **3.x**; board: **Waveshare ESP32-S3-Touch-LCD-7**
- Arduino libraries: **ESP32_Display_Panel**, **LVGL 9.5.0** (do not mix LVGL 8 and 9)
- The four files in this folder must stay together: `slot18_c4002_lcd_dashboard.ino`, `esp_panel_board_custom_conf.h`, `esp_lv_adapter_arduino.h` and `esp_lv_adapter_arduino.cpp`.
- Waveshare's official vendor example includes its custom `lv_conf.h` / LVGL setup if needed. Use the official instructions for those dependencies.

The three support files were copied from Waveshare's Apache-2.0 official example, with only `ESP_OPEN_TOUCH` switched from 1 to 0; original header SPDX license notices are retained where present. The main sketch reuses Slot-18's sensor parser and the manufacturer's LVGL adapter setup pattern.

Official docs: [Waveshare Arduino LVGL instructions](https://docs.waveshare.com/docs/ESP32/ESP32-S3/ESP32-S3-Touch-LCD-7/Arduino/Arduino-LVGL-Demo).

## One test only

1. Open the **new** `slot18_c4002_lcd_dashboard.ino` folder in Arduino IDE with the support files together.
2. Choose Waveshare **ESP32-S3-Touch-LCD-7**, correct port, 115200 serial, and the same working sensor wiring as prior tests.
3. Compile and upload. If it won't compile, stop and send the **first error**, including which library version Arduino is using. Do not change the verified old programs.
4. Look at the **Waveshare's own 7-inch screen**. It should show two distance panels, colored bars, radar state and a LIVE/WAITING link indicator. Move slowly toward/away from radar and look for the numbers to update.
5. Send a photo of the Waveshare display plus any Serial Monitor startup/error lines. The sketch prints a short `LIVE` status line every 5 seconds for diagnosis.

**Expected serial:** `LCD DASHBOARD READY; starting C4002 link...` then `RADAR BRIDGE READY, UART A 115200.`, then `LIVE:` lines. A blank/white LCD or `STOP:` is **not** a pass; provide exact evidence for the next atomic fix.

## Limits

- LCD v1 is display-only, not touch-controlled.
- No radar configuration write, environmental calibration, or reboot is commanded.
- Distance traces cannot identify a human or locate targets on an X/Y map.
- No power-supply changes requested; leave existing verified wiring.
