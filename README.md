# Slot-18 — Handheld Multi-Sensor Detection System

**Status:** IN PROGRESS — planning and software-only prototype. **Latest candidate:** v0.1.0 (development scaffold; not hardware-tested). **Last user-verified version:** none.

A modular ESP32-S3 based, handheld 7-inch touchscreen sensor instrument that can later communicate with battery-powered remote stations, render honest radar/thermal/LiDAR/GNSS measurements, and present overhead/property-map or floor-plan views.

## Initial hardware targets (not yet received)
- Waveshare ESP32-S3-Touch-LCD-7 (7-inch capacitive touch, N16R8)
- Hi-Link HLK-LD2450 24 GHz moving-target tracker
- DFRobot C4001 **SEN0609** 25 m variant
- DFRobot C4002 **SEN0691** stationary-presence radar
- Potential SC16IS752 dual UART expansion module (exact wiring/throughput unverified)

The first milestone uses **simulated readings only**. No claims of actual sensor detection, through-wall sensing, people identification, GPS accuracy, or working ESP32 firmware are made.

## First implementation
- `prototype/index.html` — desktop/mobile browser-based UI mock and demo (open the HTML file locally). Clearly marked SIMULATED.
- `prototype/core.js` — platform-neutral coordinate/presence data core with tests.
- `prototype/test_core.cjs` — Node.js built-in test runner tests (`node --test prototype/test_core.cjs`).
- `docs/` — hardware, system architecture, purchase options, diagnostic coverage, test plan, and handoff.

## Planned progression
1. Software-only radar visualization and core data contract.
2. Bring up 7-inch display and touch using Waveshare's working ESP-IDF/LVGL demonstration; use existing board drivers, no guessed pinout.
3. Integrate LD2450; then C4001 and C4002 individually and together, checking interference.
4. Add IMU, GNSS, thermal, LiDAR, battery metrics, and remote ESP32 stations incrementally.
5. Field-test four-direction coverage; consider eight directions only when proven necessary.

## Governing development standards
At each new development session first read `auxz2jz/master-instruction-library/INSTRUCTION_INDEX.md` and its mandatory global standards. Canonical library: https://github.com/auxz2jz/master-instruction-library . Also read `PROJECT_MEMORY.md`, `ROADMAP.md`, `docs/DIAGNOSTICS_AND_TESTING.md` before modifying this project. Never call an untested build VERIFIED.

## Safety and technical reality
These are research-grade prototype sensors, **not** life-safety, rescue, security-certification, or guaranteed occupancy tools. Position dots are only drawn for measurements that actually contain position. Presence-only returns sectors/range uncertainty, not invented precise person positions. See architecture and limitations docs.
