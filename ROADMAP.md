# Roadmap
Status legend: PLANNED / IN PROGRESS / CANDIDATE / VERIFIED / BLOCKED / DEFERRED.
A test passing does not make a candidate user-VERIFIED.

## Milestones
- v0.1.0 [CANDIDATE] Standalone browser radar UI + simulated telemetry, normalized measurement core, JSONL diagnostic export, simple guided visual testing.
- v0.2.0 [IN PROGRESS; SOURCE CANDIDATE] ESP32-S3 7-inch touchscreen using official Waveshare display port, ESP-IDF + LVGL v9, test controls and simulated moving dot. Source in firmware/esp-idf; CI and physical flash/test are separate gates. Bench power from regulated wall-outlet USB supply. See docs/FIRST_HARDWARE_TEST.md.
- v0.3.0 [PLANNED] Single LD2450 UART: framed packets, unit/axis verification, live XY dots, disconnected state, logs and replay tests.
- v0.4.0 [PLANNED] C4001 SEN0609 driver; range, motion, presence with truthful uncertainty; confirm UART/I2C hardware details.
- v0.5.0 [PLANNED] C4002 SEN0691 driver; test still/sitting detection and false positives.
- v0.6.0 [PLANNED] SC16IS752 bridge if needed; test sustained 256k serial LD2450 directly on internal UART, slower sensors on bridges; validate on actual board.
- v0.7.0 [PLANNED] Sensor fusion without inventing position; configurable mounts, headings, overlap, interference testing.
- v0.8.0 [PLANNED] BNO085 IMU/compass + magnetometer calibration + walking/stationary indications. Static obstacles/body masking explicitly tested.
- v0.9.0 [PLANNED] One compact-battery remote node using ESP-NOW/local Wi-Fi; authenticated telemetry, offline handling, battery measurement. Test **direct** links before adding forwarding. Main display stays on wall USB power until portable phase.
- v1.0.0 [PLANNED] Five remote nodes; manual survey coordinates or GNSS; maps show station uncertainty and measurement uncertainty. Add separately tested **multi-hop mesh relaying/reroute** so nodes outside base range can reach it through powered neighbors, with last-seen, battery and routing diagnostics.
- later [PLANNED] Four-direction tower, up to 12 active radars; test before expanding to eight/24; optional thermal (MLX90640), LiDAR floor plan, camera, 3D radar, RTK, air-quality and environment sensors.

## Out of scope for initial milestone
- Actual 7-inch display firmware, on-device APK, outdoor waterproofing, precise people location from range-only reports.
- Guarantee of detection through walls, floors, cars or metal; human-vs-animal classification.
- Automated alarms used as safety-critical evidence.

## Purchasing sequence
1. Touchscreen + USB power, ONE LD2450 (optional C4001/C4002 if purchased together).
2. SC16IS752 only after direct UART and board interface review; compatible cables and logic-level/power budget.
3. GNSS + calibrated orientation + small ESP32 node + safe battery solution; one node first.
4. LiDAR/thermal and additional sector copies only following field results.

## Hardware bring-up checkpoint
- First: run unmodified Waveshare display/touch demo and record user-observed behavior.
- Next: flash Slot-18 v0.2.0 and verify top SIMULATED ONLY banner and touch controls.
- Then: LD2450 UART v0.3.0; no additional UART bridge needed for first screen-only test.

## Test strategy
See docs/DIAGNOSTICS_AND_TESTING.md. At each milestone write real compile/test evidence and a handoff checkpoint. Never move LAST VERIFIED without user testing.

## Android and web extension roadmap (PLANNED, no source or APK created)
- F-020 / F-025: Design a versioned sensor API preserving real-vs-simulated state, actual measurement capabilities, station identities, last seen and uncertainty.
- F-020: ESP32 direct offline Wi-Fi access point and read-only browser radar view, using bounded authenticated WebSocket-like telemetry. Prove local mode first.
- F-021 / F-022: Independent native Android implementation: use Wi-Fi for full radar graphics, BLE for discovery/pairing/settings/small telemetry, with independently verified APK.
- F-023: Router/hotspot local-network mode and multiple authorized viewers; test wireless coexistence with eventual remote station mesh.
- F-024 / F-027: Optional remote internet viewing over authorized encrypted VPN gateway or outbound relay; no direct open internet access by default.
- F-028: Possible installable browser PWA for phone/PC; native Android app still preferred when robust BLE is needed.
- All new client work follows docs/REMOTE_ACCESS_AND_CLIENTS.md and shared feature/interface docs. No change to existing untested v0.2.0 firmware source.

## Battery/case scope — new user-defined 2/4-slot protected LiPo cartridge system (PLANNED)
- POWER-01: Define protected 1S pouch LiPo modular cassette, mechanically keyed recessed spring / blind-mate contacts, mechanical CAD and fit/short-test without live LiPo cells.
- POWER-02: Pick per-pack charger/protection plus per-slot independent ideal-diode/MUX power sharing; calculate 5V power budget and require ANY installed adequate pack to support worst-case load.
- POWER-03: Prototype remote 2-bay hot-swap system on bench supplies/electronic load first, then correctly protected packs, testing swap in/out, reverse current, brownout, faults, and current/temperature; independent charging dock.
- POWER-04: Scale validated architecture to handheld 4-bay removable case, preserving wall USB testing and regulated 5V device input (not Waveshare PH2.0 multi-pack wiring).
- POWER-05: Add per-bay SoC, measured runtime estimate, pack aging/SoH where supported and battery diagnostics to remote telemetry and handheld/web/Android UI.
- POWER-06: Evaluate optional in-device independent multi-channel charging with safe input/current/thermal limits after standalone dock works.
- Design specification: docs/HOT_SWAPPABLE_LIPO_POWER_SYSTEM.md. No milestones automatically coded until earlier firmware bring-up verified.
