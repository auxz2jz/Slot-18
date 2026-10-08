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

## Proposed larger 18650 packs / unattended runtime (UNDER EVALUATION)
- Compare protected four-cell 18650 cartridges with earlier LiPo pouch concept; user may want 2x four-cell modules per remote station and multiple cartridges on handheld.
- Define whether hot-swapping is required at the **module/cartridge** level or each individual 18650; an ordinary parallel holder is not independently cell hot-swappable.
- Use measured live ESP32 radar/mesh/display currents and user-selected runtime target to select capacity, regulator, contacts, BMS/fuses/charging, maintenance service plan and printed enclosure size.
- See docs/18650_MULTICELL_POWER_SIZING.md. Keep existing v0.2.0 firmware unchanged and all power implementation as PLANNED until measured/tested.

## Confirmed in-device USB/solar charging and full-cartridge hot-swap — DESIGN ONLY
- POWER-07: Choose protected serviceable four-cell 18650 cartridges, whole-pack hot swapping only. Cell service/replacement happens with the cartridge fully removed and evaluated; matched-cell/balancing constraints documented.
- POWER-08: Add USB-C/DC external power input to BOTH remote nodes and main handheld, with system-load-priority power path, input current limits and safe automatic changeover from batteries without reboot.
- POWER-09: Add independent per-pack in-device lithium charger/protection/thermal paths: remote 2, handheld 2 initially and possible 4; prevent charging one pack from another and enforce stable regulated 5V device rail.
- POWER-10: Add optional PV input via correct solar MPPT/input DPM controller and charge from surplus energy while equipment runs, including realistic daily energy budget, shading and hot weather protections.
- POWER-11: Provide external USB, solar, battery-source, per-pack charging/current/temperature/fault and estimated runtime telemetry, exported over remote mesh to touchscreen and Android/web.
- Begin with actual measured currents and protected bench-supply emulation before lithium cells; preserve v0.2.0 firmware candidate with no modifications. Detailed plan: docs/USB_SOLAR_MULTIPACK_POWER_ARCHITECTURE.md.


## Device-side backplane — planned gated hardware integration
- BP-01 Confirm IP5328P exact board pinout, BOOST 5V source (always 5V vs PD switched voltage), independent 5V charge input, output auto-shutdown and charging behavior. If not dock-compatible, change power-bank choice rather than forcing unsafe wiring.
- BP-02 Develop remote 2-cartridge protected 5V selector with eFuse/hot-swap/current-limit, auto power switching, single-cartridge full-load rating; TPS2121 is a candidate reference, not a chosen purchasable ready-to-plug board.
- BP-03 Expand selector to handheld 2-4 cartridges with validated path ratings and no pack backfeed. Keep individual cartridge channels separate, preserve pack identity and SoC limitations.
- BP-04 External USB-C/PD and solar-regulated 5V supply **directly powers device**, with priority MUX over cartridge backup and independent per-cartridge charge-enable/power budget. Check IP5328P charging disables boost and unexpected solar or wall removal does not reboot the ESP32.
- BP-05 Bench tests of source insertion/extraction, sleep/wake and delayed output restoration, true USB-C PD vs 5V-only pogo charging, solar fluctuation, short/thermal protection, telemetry, and 5V rail with real loads. Full plan: docs/DEVICE_SIDE_POWER_BACKPLANE.md. Software candidate stays unchanged.
