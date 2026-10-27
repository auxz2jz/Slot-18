# Slot-18 — Handheld Multi-Sensor Detection System

**Status (2026-10-27):** PHYSICAL BENCH BRING-UP IN PROGRESS. Waveshare factory display/touch user-verified. External CJMCU-752 I²C recognition, register tests and dual UART internal loopback passed; C4002 external UART **NOT VERIFIED**. **Firmware candidate:** ESP-IDF v0.2.0, CI build failed (missing esp_random.h; not fixed here) and not user-verified. **Browser candidate:** v0.1.0 automated core tests passed but visual UI not user-verified. Full recoverable status: [NEXT_CHAT_HANDOFF.md](NEXT_CHAT_HANDOFF.md).

A modular ESP32-S3 based, handheld 7-inch touchscreen sensor instrument that can later communicate with battery-powered remote stations, render honest radar/thermal/LiDAR/GNSS measurements, and present overhead/property-map or floor-plan views.

## Hardware targets and actual bench inventory (2026-10-27)
- Waveshare ESP32-S3-Touch-LCD-7 Rev1.2 (7-inch capacitive touch, N16R8) — **received, factory touchscreen verified**
- Hi-Link HLK-LD2450 24 GHz moving-target tracker — **received, not tested**
- DFRobot C4001 **SEN0609** 25 m variant — planned, not confirmed received
- DFRobot C4002 **SEN0691** stationary-presence radar — **received, powered at ~4.9V, UART link not yet verified**
- Two CJMCU-752 SC16IS752 dual UART expansion modules — **received**; first board passed address 0x4D scan and internal UART tests. Real radar serial link still unresolved.

The project software's first milestone uses **simulated readings only**. Real sensor data, custom firmware UI and end-to-end radar communication remain unverified, but Waveshare **factory** display and touch were physically verified on 2026-10-27. An ESP-IDF v0.2.0 source candidate exists under firmware/esp-idf; previous CI failed on an esp_random.h include. Compilation and physical testing are separate gates. Arduino serial-only diagnostics live on the user's Windows PC, not in production firmware.

## First implementation
- `prototype/index.html` — desktop/mobile browser-based UI mock and demo (open the HTML file locally). Clearly marked SIMULATED.
- `prototype/core.js` — platform-neutral coordinate/presence data core with tests.
- `prototype/test_core.cjs` — Node.js built-in test runner tests (`node --test prototype/test_core.cjs`).
- `docs/` — hardware, system architecture, purchase options, diagnostic coverage, test plan, and handoff.

## Planned progression
1. Software-only radar visualization and core data contract (v0.1.0 candidate, unit tests passed).
2. Use the prepared v0.2.0 source in firmware/esp-idf, after checking Waveshare's unmodified sample. Compile and user-test the 7-inch LVGL touchscreen. Follow docs/FIRST_HARDWARE_TEST.md; no guessed pinout.
3. Integrate LD2450; then C4001 and C4002 individually and together, checking interference.
4. Add IMU, GNSS, thermal, LiDAR, battery metrics, and remote ESP32 stations incrementally.
5. Field-test four-direction coverage; consider eight directions only when proven necessary.

## Hardware and firmware entry points
- [Firmware v0.2.0 candidate](firmware/esp-idf/README.md) — ESP-IDF, LVGL v9, simulated target and touch test.
- [First hardware test guide](docs/FIRST_HARDWARE_TEST.md) — PC flashing, safe USB wall power, and later one LD2450.
- [Project memory](PROJECT_MEMORY.md) and [roadmap](ROADMAP.md) keep evidence and checkpoints.

## Governing development standards
At each new development session first read `auxz2jz/master-instruction-library/INSTRUCTION_INDEX.md` and its mandatory global standards. Canonical library: https://github.com/auxz2jz/master-instruction-library . Also read `PROJECT_MEMORY.md`, `ROADMAP.md`, `docs/DIAGNOSTICS_AND_TESTING.md` before modifying this project. Never call an untested build VERIFIED.

## Safety and technical reality
These are research-grade prototype sensors, **not** life-safety, rescue, security-certification, or guaranteed occupancy tools. Position dots are only drawn for measurements that actually contain position. Presence-only returns sectors/range uncertainty, not invented precise person positions. See architecture and limitations docs.
