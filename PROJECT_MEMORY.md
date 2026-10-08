# Project Memory / Recovery Checkpoint
Date: 2026-10-07 (America/Los_Angeles)
Repository: auxz2jz/Slot-18
Canonical master rules: https://github.com/auxz2jz/master-instruction-library

## Status
- Project: Handheld Multi-Sensor Detection System
- Current task: initialize permanent project and start a software-only radar visualization/data-contract prototype
- Candidate: v0.1.0 — software-only demonstration, NOT a flashed ESP32 build
- Last user-verified version: NONE
- Latest legitimate artifact: repository main branch, pending initial project commit and explicit testing
- Hardware received/tested: NONE
- Hardware purchases: planned only; do not assume orders placed
- User direction: begin before hardware arrives; user will advise when available
- Cross-platform: browser prototype is a design harness; ESP32 firmware is the intended target. No Android/Windows app started.

## Confirmed design preferences
- Handheld 7-inch touchscreen ESP32-S3; operator may raise antenna array on a pole above head.
- Start with ONE each LD2450, DFRobot C4001 25m SEN0609, and C4002 SEN0691; evaluate SC16IS752 dual UART expansion.
- Later four cardinal directions / up to 12 radar modules, expandable to eight directions / up to 24 only after field coverage tests.
- Moving person X/Y tracking where available; stationary people / sleeping presence; approximate distance; overhead visualization.
- Five or more battery-powered remote ESP32 sensor stations possible, with GPS and heading/tilt sensors; map overlay and room/floor-plan view.
- Modular capability-based adapters for radar, thermal, GNSS/RTK, IMU/compass, LiDAR/depth, PIR, camera, magnetic, vibration, ultrasonic and door contacts.
- Prefer Amazon comparison listings, budget-conscious staged purchases, open-source tooling where practical.
- No internet or cloud required for local mode.
- Battery constraint clarified 2026-10-07: **No large/heavy 12V batteries**; main handheld starts USB-C, future 5,000-10,000mAh compact USB power bank or appropriate single 3.7V small battery for Waveshare onboard PH2.0 (manufacturer suggests <=2,000mAh). Remote nodes use compact protected 18650/21700 or flat 1S LiPo with correct charging, voltage regulation, and voltage monitoring. Mesh relay nodes stay awake and may have shorter runtime; measure current before final sizing. Full research in docs/HARDWARE_AND_SENSORS.md.

## Technical constraints / warnings
- All three candidate radar models share 24 GHz region; mutual interference must be measured.
- Indoor occupancy cannot be guaranteed; through-wall/through-floor detection unreliable and not suitable for emergency/security certainty.
- A person/hand holding the unit can shadow radar; walking induces apparent target motion; magnetometers require calibration.
- LD2450 track X/Y does not provide independent height. C4001 and C4002 presence/range must NEVER be plotted as exact dots.
- GPS locates outdoor stations, not radar targets; stationary GPS does not determine orientation; an IMU without a calibrated heading reference can drift.
- Screen's UART connector/jumper ports are shared; no untested assertion of available independent serial ports.
- The C4001 documentation mentions UART + I2C but exact SEN0609 wiring/firmware must be verified before relying on I2C.
- GNSS accuracy is not submeter by default; RTK requires correction service/base and sky view.
- All radars active continuously may drain a small battery rapidly. Power budget is not yet measured.
- Prototype is not a reliable life-safety or emergency location device.

## Actual implementation / verification
- Initial project scaffold + browser simulator created as candidate v0.1.0.
- Source files: prototype/index.html, style.css, core.js, app.js, test_core.cjs.
- Software build/automated-test status: not yet verified by user.
- Real display, radar, GPS, compass, mapping, firmware and battery runtime: NOT IMPLEMENTED.
- Known bugs/failed approaches: none documented yet; do not infer success before running tests.
- Diagnostic/test protocol in docs/DIAGNOSTICS_AND_TESTING.md.

## Initial automated test result (not user hardware verification)
- GitHub main candidate commit: bbae81bf6b472ec954a6387a59fe5601c963610f
- GitHub Actions workflow: https://github.com/auxz2jz/Slot-18/actions/runs/37724357604
- Result: SUCCESS; Node syntax checks for core.js/app.js and unit tests passed.
- Test scope: basic normalization, coordinates, synthetic target types and bad input. Browser visual operation and actual ESP32 hardware have NOT been user-verified.
- Attempted local clone to test in container failed because container could not resolve github.com; used the repository's GitHub Actions result instead.

## Exact next action
1. User or developer opens prototype/index.html in browser, follows docs/DIAGNOSTICS_AND_TESTING.md, and records PASS/FAIL and visual findings.
2. When screen arrives, load vendor Waveshare ESP-IDF/LVGL demo and establish verified backlight/touch baseline; keep separate from simulator status.
3. Add LD2450 physical driver first and test orientation, range, packet decodes, and diagnostics. Then C4001/C4002 and UART expansion as indicated by physical test evidence.
