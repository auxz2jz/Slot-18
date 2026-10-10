# Project Memory / Recovery Checkpoint
Date: 2026-10-07 (America/Los_Angeles)
Repository: auxz2jz/Slot-18
Canonical master rules: https://github.com/auxz2jz/master-instruction-library

## Status
- Project: Handheld Multi-Sensor Detection System
- Current task: prepare v0.2.0 ESP32-S3 physical screen bring-up as an **unverified candidate** (screen simulator first), then integrate one LD2450 radar only after display/touch verification.
- Candidates: browser v0.1.0 (basic unit tests passed); firmware v0.2.0 (source committed, firmware CI build pending, not flashed and not user-tested).
- Last user-verified version: NONE
- Source artifact: v0.2.0 ESP-IDF display simulator source committed at 7ef78bcfb7ffac51acfb310cc08c679223b583d1. Existing v0.1.0 browser simulator stays intact; no user-confirmed baseline.
- Hardware received/tested: NONE confirmed. User said to ASSUME a Waveshare 7-inch ESP32-S3 and one radar are available for planning; not proof of hardware in hand.
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
- Wireless plan: start ESP-NOW/local Wi-Fi for direct links, then evaluate **true multi-hop/daisy-chain mesh** (Espressif ESP-Mesh-Lite or validated routing design) so out-of-range stations can relay through nearby powered peers to the main display. Preserve originating node IDs and timestamps, support automatic rerouting/offline buffering where feasible, and measure relay battery costs. Multi-hop mesh has NOT been implemented or tested.
- Power plan updated 2026-10-07: **Initial main 7-inch ESP32 touchscreen tests are powered from a household wall outlet via a suitable regulated USB 5V wall adapter and USB cable**; NOT from a battery during bench testing. Use an adapter with adequate current for the installed load, tentatively 5V/3A, and confirm board USB power requirements before hookup. No mains wiring to ESP32 or sensor pins. Later handheld stage uses small rechargeable power (USB-C 5,000–10,000mAh bank or a compatible protected single-cell 3.7V LiPo with Waveshare's manufacturer-recommended charge limits). Remote sensor/mesh stations will eventually use compact protected 18650/21700 or flat 1S LiPo plus appropriate charger/regulator/monitor; no bulky 12V battery. Mesh relays cannot deep-sleep while forwarding. Exact Amazon search links and cautions are saved in docs/HARDWARE_AND_SENSORS.md.

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
- Initial project scaffold + browser simulator created as candidate v0.1.0; full v0.2.0 ESP-IDF project started under firmware/esp-idf based on the official Waveshare ESP-IDF LVGL v9 port.
- Source files: prototype/index.html, style.css, core.js, app.js, test_core.cjs.
- Software build/automated-test status: not yet verified by user.
- Real display operation, real radar inputs, GPS, compass, mapping and battery runtime: NOT VERIFIED/NOT IMPLEMENTED. ESP32 firmware SOURCE now exists, but no real-data UART radar driver is implemented.
- Known bugs/failed approaches: none documented yet; do not infer success before running tests.
- Diagnostic/test protocol in docs/DIAGNOSTICS_AND_TESTING.md.

## Initial automated test result (not user hardware verification)
- GitHub main candidate commit: bbae81bf6b472ec954a6387a59fe5601c963610f
- GitHub Actions workflow: https://github.com/auxz2jz/Slot-18/actions/runs/37724357604
- Result: SUCCESS; Node syntax checks for core.js/app.js and unit tests passed.
- Test scope: basic normalization, coordinates, synthetic target types and bad input. Browser visual operation and actual ESP32 hardware have NOT been user-verified.
- Attempted local clone to test in container failed because container could not resolve github.com; used the repository's GitHub Actions result instead.

## v0.2.0 screen firmware checkpoint (2026-10-07)
- Candidate source commit: 7ef78bcfb7ffac51acfb310cc08c679223b583d1.
- First firmware CI run: https://github.com/auxz2jz/Slot-18/actions/runs/37726264848 FAILED in dependency resolution (ESP-IDF 5.4.2 incompatible with esp_lvgl_adapter ^0.5.2 requiring ESP-IDF >=5.5). This is an environment/dependency mismatch, not yet a source compile assessment.
- Fix: switched CI image to pinned ESP-IDF 5.5.2 and updated setup docs, preserving original browser simulator; current CI candidate: https://github.com/auxz2jz/Slot-18/actions/runs/37726404321 (started; check actual result before claiming build success).
- No physical screen test, radar input, or user-verified baseline yet.
- Uses official Waveshare port + LVGL 9 and a self-contained simulated screen: Facing / Pause / Test / PASS / FAIL / LOGS, NVS bounded persistent serial diagnostics.
- First physical wiring assumptions deferred: GPIO43/44 is UART0 shared between board UART1 and UART2 via switch, so logging and physical radar connection conflict unless deliberately separated. Board 05_UART_Test default GPIO4/5 unsuitable with LCD/touch.
- Detailed procedure: docs/FIRST_HARDWARE_TEST.md.

## Exact next action
1. Check v0.2.0 ESP-IDF CI build conclusion and fix any actual build failure once using evidence; update this checkpoint.
2. On actual board, first flash **unaltered vendor Waveshare display demo** to establish hardware touch baseline, then build/flash Slot-18 firmware v0.2.0 and use the guided UI tests. Record user-observed PASS/FAIL and serial logs.
3. After user confirms display and touch, add real LD2450 UART driver in a separate candidate, verify pinout, power, UART conflicts and decoded XY measurements. Preserve last confirmed state.

## Companion interfaces and connectivity — requested 2026-10-07 (DESIGN ONLY)
- User explicitly states neither the program nor physical device has been tested yet. Firmware v0.2.0 remains an unverified candidate; no Android APK or networked website exists.
- User requests an Android app plus web browser dashboard for viewing and appropriately controlling the main ESP32-S3 sensor system, over nearby BLE (lightweight pairing/settings/status), direct ESP32 Wi-Fi with no internet, shared local Wi-Fi router/hotspot, and optional remote internet access only when provisioned and connected.
- ESP32-S3 supports BLE only, not Classic Bluetooth. Prefer Wi-Fi for full real-time maps and WebSocket updates; BLE is best for provisioning and lower-rate readings. Simultaneous BLE, Wi-Fi SoftAP/STA and ESP-NOW/mesh must be performance-tested.
- Secure internet access needs explicit VPN-capable gateway or outbound TLS relay and authentication; internet connectivity alone is insufficient. Avoid exposing unauthenticated ESP32 HTTP ports. ChatGPT does not automatically have direct device access.
- Added permanent cross-platform planning at docs/REMOTE_ACCESS_AND_CLIENTS.md, shared/FEATURE_CATALOG.md, shared/DATA_FORMATS_AND_INTERFACES.md, android/README.md, web/README.md. Android/web versions and verification must be tracked independently from ESP-IDF and browser simulator.
- Next physical step unchanged: finish main touchscreen firmware and single-radar tests before implementing network services. The current additions are documentation only.

## Hot-swappable pouch LiPo requirement — 2026-10-07 (DESIGN ONLY)
- User prefers **flat silver lithium-polymer pouch batteries** (yellow/orange Kapton-taped), not cylindrical cells or large 12V batteries, for both handheld and remote stations. Power source for **initial main screen tests remains USB wall outlet**, so there is no new requirement to wire lithium cells before firmware testing.
- Each remote station should have **2 independent removable hot-swap battery slots**. Main handheld should have **4 removable slots (possibly near 4 corners)**. Device should operate on any one adequate pack, any combination of slots, and continue to function when another battery is removed/replaced; verify real power/peak-load capability first.
- Each pack should be protected and in a safe keyed 3D printed cassette with insulated recessed/rated blind-mate or pogo power contacts, optional thermistor/pack ID. Manage power with reverse-current-blocked ideal-diode OR/mux paths; NEVER directly parallel cells or plug 2–4 cells into Waveshare single-cell PH2.0 charge interface.
- Charging: initial preferred design is external removable pack charging dock with a fully independent correct 1S LiPo charger channel per battery. Optionally integrated independent in-device chargers after power-path/thermal/electrical validation; single BQ24074 breakout only manages one battery and does NOT provide multi-bay hot swapping.
- Remote station telemetry must include battery present count/individual bay ID, SoC%, voltage, current/temperature where measured, SoH (when instrumented), per-pack and overall estimated runtime, low battery, charging state, health faults, source activity, safe-to-remove. Relay these readings to 7in screen and later Android/web. Time-to-empty is ESTIMATE, calibrated from measurements, not simply inferred from voltage.
- Separate doc saved: docs/HOT_SWAPPABLE_LIPO_POWER_SYSTEM.md with power topology, Amazon-only model searches, mechanical design and test milestones. This addition is **planning only**: no charger board chosen/bought, no battery PCB/CAD, no code changed and no physical result verified.

## Battery capacity exploration — 2026-10-07 (NOT FINAL OR VERIFIED)
- User is now considering replacing pouch-cell concept with standard cylindrical 18650 holders, with **four cells per side on a remote station (two 4-cell modules, 8 cells total)** and multiple four-cell modules on the handheld. Wants longer unattended operation and easier individual cell replacement.
- Energy estimate using 3,000mAh/3.6V cells and 80% usable: 4 cells ~35Wh, 8 ~69Wh. At hypothetical 1.5–2.5W remote sensor/mesh load, 8 cells ~28–46 hours. The handheld's Waveshare display alone ~2.25W, so it needs more power than a small ESP32 sensor station. All numbers are sizing estimates, NOT measured data.
- Critical distinction: a simple 1S4P PARALLEL spring holder ties cells together, and hot insertion/removal of individual mixed-charge cells can produce unsafe surge/equalization currents; a SERIES pack needs a proper BMS and cannot remain energized with a cell removed. Recommend whole **protected four-cell cartridge** as the hot-swappable unit unless user chooses higher-complexity per-cell isolated paths. Preserve pouch alternative pending selection; do not buy/finalize pack PCB or CAD.
- Sizing, per-cell/pack safety, Amazon-only searches and layout considerations documented at docs/18650_MULTICELL_POWER_SIZING.md. No current firmware/hardware baseline changed.

## Clarified battery replacement, USB and solar operation — 2026-10-07 (user confirmed; PLANNED)
- **Entire four-cell 18650 cartridge** is the hot-swappable unit. Individual cells must NEVER be removed/inserted while the cartridge is in the handheld or remote device. After removing a whole cartridge, it can be opened and its cells serviced offline, subject to cell matching, charge equality and battery safety checks. A generic parallel holder does NOT make unequal single-cell replacement safe, even offline; prefer replacing matched sets or build explicit individual cell isolation.
- Remote station design: 2 independent removable packs x 4 cells. Handheld: 2 such packs initially, potentially expandable to 4. Each pack must independently sustain entire worst-case device load when it's the only one installed; pack identities and SoC/health retained.
- **Both station and handheld must include USB/wall power input that directly powers electronics AND charges installed cartridges when power is sufficient**; not merely a separate charging dock. **Both may accept an optional solar panel** using a proper solar charge management input. Prefer external source to system load; use available surplus to charge each cartridge independently. The current wall-USB-only initial screen test remains unchanged.
- Require independent charger/protection/thermal monitoring for each removable multi-cell pack plus separately rated protected hot-swap power paths, OR/MUX, stable regulated 5V system rail, USB power/input negotiation if needed, PV MPPT/dynamic power control, adapter/solar source selection/backfeed protection, per-pack fuel gauges and fail-safe hardware transitions.
- Evaluation IC: TI BQ25798 1–4 **series-cell** charger with solar MPPT and NVDC power path manages ONE pack, not 2–4 distinct removable cartridges automatically. Adafruit BQ24074 also only a single 1S pack with non-5V LOAD. Neither is a proven turnkey 2/4-cartridge solution.
- Existing removable-pouch and external-charging-only alternatives remain as historical design options; priority now is in-device charging and live USB/solar operation with removable four-cell 18650 cartridges. Detailed document: docs/USB_SOLAR_MULTIPACK_POWER_ARCHITECTURE.md. Nothing has been physically built or user-tested.

## Architecture preference — modular power boards (2026-10-07; DESIGN ONLY)
- User asks to combine separate functional modules (per-cartridge Li-ion charger, common USB/solar input power management, isolated per-cartridge OR/MUX output stage and final shared regulated voltage output) rather than finding a single expensive all-in-one 2/4-pack controller. This is adopted as preferred design direction pending electrical feasibility/tests. One cartridge remains a protected, serviceable four-18650 candidate pack; only full packs are hot swapped.
- Same rated output voltage shared by all installed packs does NOT mean all pack terminals can be directly paralleled, or that current sharing automatically equalizes. Available Wh and supported current can increase; drawn amperage is set by device demand/regulator. Independent charger per 1S4P module recommended for easiest simultaneous in-device charging, with option of a single charger sequentially switched between cartridges if supported with safety interlocks.
- Maintain pack BMS/protection as well as device slot reverse-current blocking, hot plug inrush/fault control and load priority; a normal TP4056 does not provide a true system load-share path. External wall/solar input is selected and power-budgeted, powers system directly with surplus charging each removable cartridge. Need bench tests before selecting modules.
- Documentation updated in docs/USB_SOLAR_MULTIPACK_POWER_ARCHITECTURE.md. This was a **documentation-only** edit; current firmware remains untested.

## Cartridge dual charging convenience — 2026-10-07 (PLANNED)
- User wants **each removable four-cell 18650 cartridge** chargeable via its own USB-C port, via a slide-in powered dock with spring charging contacts, and while installed in a handheld/remote station through that device's wall/USB/solar charging path. Ideally all feed the same correctly rated pack charger/protection circuit through isolated/selected inputs, never directly tie independent input sources together or double-charge via separate uncontrolled chargers.
- Cartridge USB-C does not imply fast-charge PD; choose matching input current, charger thermal and pack capacity (~12Ah nominal for 1S4P using 3000mAh cells). Charge rate 1A means >12h for a depleted pack. Protect/recess docking and output terminals; whole cartridge hot swap stays separate from individual cell service.
- Spec recorded at docs/USB_SOLAR_MULTIPACK_POWER_ARCHITECTURE.md. Feature is design only; no firmware/PCB/CAD validation.


## Device-side power backplane phase — 2026-10-07 (PLANNED ONLY)
- Following user's direction to progress from per-cartridge IP5328P candidate into the DEVICE, added separate device-side power backplane design: docs/DEVICE_SIDE_POWER_BACKPLANE.md.
- Remote sensor station: two independent 5V cartridge power inputs through individual fault/reverse-current protection and a suitable 2-source MUX/OR stage (TI TPS2121 candidate). Handheld: two initially, expand to four with per-source isolation, properly sized combiner. Separate external USB-C and regulated optional solar source take load priority via another suitable selector. Each cartridge needs independent protected charging input path, source-current budgeting, and diagnostics/SoC where reliably measured.
- CRITICAL verified datasheet fact: **standard IP5328P boards disable boosted 5V output while charging**; do not assume a USB-C charger can supply display through cartridge in charging state or 5V pogo output always exists. Charge and load paths must be separate in the DEVICE; hardware must survive loss of wall/solar even if all cartridge power-bank ICs are asleep/charging. Need benchtop transition test or different pack charger/boost architecture. TPS2121 quick internal switching is not enough to guarantee always-on supply with unavailable inputs. IP5328P may also go into low-load standby. Verify exact board's independent power in/out pads before designing pogo interface.
- No software change / user verified hardware remains NONE. Continue bench bring-up with regulated wall USB and test electrical power with current-limited supplies before lithium cell use.


## FINAL HANDOFF CHECKPOINT — 2026-10-09 (authoritative current status)
- User stopped ongoing Waveshare / BMS research ("Stop you're in a loop") and explicitly requested save everything to GitHub for a new chat to take over. **No further product research or coding was performed**; this is documentation-only.
- Start future session at **`NEXT_CHAT_HANDOFF.md`** in repository root. It summarizes user-confirmed requirements, both 5V-regulated and raw-pack OR architectures under consideration, component caveats, physical tests still required, hardware/product/CAD source inventory, safety constraints, and the *single* next action for each possible resume track. Older scattered speculative component recommendations are NOT purchase approval.
- Firmware: browser v0.1.0 automated core test PASS; ESP-IDF screen source v0.2.0 CANDIDATE, user has tested NONE; latest ESP-IDF build run `37726404321` **FAILED**, because `firmware/esp-idf/main/main.c:182` calls `esp_random()` without explicit `#include "esp_random.h"`. Earlier 5.4.x ESP-IDF adapter incompatibility was addressed by the 5.5.2 workflow update. Neither issue was patched here. No physical hardware working state verified.
- Amazon-only is user preference for actual purchasing. No battery module, combined OR circuit, case, ESP32 device, sensor, APK, or final schematic has been physically verified or definitively selected.
- Entire four-18650 1S4P cartridge is hot-swap unit. Separate outside USB-C/cradle charging vs charging installed through dock contacts. Remote two cartridges, handheld 2 initially possibly four, independent protection and charger per pack; 5V device rail and optional regulated solar, uninterrupted switching *not yet achieved*. Do not infer user's transient reference to 4S1P is a configuration change.
- Exact next action now: **STOP**, allow new chat to open and read NEXT_CHAT_HANDOFF.md; resume hardware investigation or targeted IDF build correction only after new user instruction.
