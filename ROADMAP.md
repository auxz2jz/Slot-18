# 2026-11-10 — Phase A C4002 read-only configuration audit candidate CREATED

- User said **Continue** after approving future calibration/sensitivity features. Implemented next atomic safe stage: `diagnostics/arduino/slot18_c4002_settings_audit/slot18_c4002_settings_audit.ino`, GitHub commit `5fb1cfb371197354672731199ad7db2c28c12366`. User-verified continuous radar monitor `diagnostics/arduino/slot18_c4002_continuous/slot18_c4002_continuous.ino` remains untouched (previously 17m8s live continuous, last cumulative 1084 good/0 errors).
- **STATUS: CANDIDATE, NOT COMPILED / NOT RUN ON USER HARDWARE.** Source audits six read-only queries via proven SC16IS752 I2C 0x4D UART A 115200 bridge: 0x82 software version, 0x87 gate0 motion sensitivity group, 0x87 gate1 presence sensitivity group, 0x86 distance min/max, 0x84 disappear delay, 0x66 resolution. Each query sends `FRAME_TYPE_READ_REQUEST=0x01`, waits max 2500ms for *matching* checksum-valid 0x03 response (and correct gate for sensitivity). Counts unsolicited 0x04 radar notifications; prints raw payload, parsed values, pass/fail and error totals. Does **not** send C4002 setter/reset/calibration/write requests; initializes *bridge* registers only as in known-good sketch. This is not yet a persistent configuration backup; user must capture output as original setting snapshot.
- **Next user action (when available):** open raw https://raw.githubusercontent.com/auxz2jz/Slot-18/main/diagnostics/arduino/slot18_c4002_settings_audit/slot18_c4002_settings_audit.ino . Arduino IDE: select proven Waveshare ESP32-S3-Touch-LCD-7 board COM3, Flash16MB/appropriate 16MB partition/PSRAM, Serial Monitor 115200, replace old editor code completely; Verify/Upload and copy full output from one-shot audit, especially six query results and AUDIT COMPLETE. No wiring changes or physical calibration needed. If compile error, resolve first from exact build output; do not assume sketch compiled. Reflash continuous original afterward to resume continuous readings.
- **Potential caveat:** reported sensitivity fields are manufacturer threshold-group labels (LOW/MID/HIGH/CUSTOM), not yet experimentally mapped to trigger sensitivity. No operator calibration/threshold write without confirmed baseline and explicit user action. Even after reads pass, settings reliability must be physically validated.
- Source/plan: `docs/C4002_CALIBRATION_AND_SENSITIVITY_PLAN.md`, marked Phase A candidate. Other undone: display/touch actual custom UI, C4002 physical calibration, LD2450. Power discussion deferred at user's request. Next coding phase after audit: single operator-confirmed setter with backup & readback (Medium reasoning; High only if protocol errors).

---

# 2026-11-10 — Planned controls for C4002 calibration / sensitivity (not yet implemented)

- User: believes hardware is working, prefers more physical testing later; wants **calibration and sensitivity adjustment available in Slot-18 programming**. User did not request an immediate sensor calibration or unauthorized device setting write.
- **Plan added**: `docs/C4002_CALIBRATION_AND_SENSITIVITY_PLAN.md`, commit `6a64b2e578637e0c561f2270b7319bdebe1131b1`. DFRobot vendor library confirms separate motion/presence sensitivity groups (Low/Medium/High/Custom), operator-started environmental calibration, min/max detection range and disappearance delay, and advanced per-gate thresholds. Plan for read-only settings audit, verified readback, per-setting confirm/write/rollback, then integrate into future Waveshare touchscreen after display driver bring-up.
- **No working sketch modified**; user-tested continuous C4002 serial monitor remains known-good baseline. **No calibration/sensitivity control tested or implemented yet**. Manufacturer recommends unoccupied area when calibrating; presence during 17min long-term test could reflect clutter, cats or metal reflections, not confirmed human presence through wall. User has deferred power supply discussion.
- **Next action once ready**: non-destructive read-only settings snapshot through existing SC16IS752 UART A bridge (DFRobot native UART library requires adapter), then one-at-a-time control implementation plus guided physical tests. Do not silently factory reset or change UART baud.

---

# 2026-11-10 — USER-VERIFIED replacement C4002 live UART test PASSED

**New user-provided Arduino Serial Monitor output:** `Slot-18 C4002 LIVE READ-ONLY TEST (25 seconds)` started normally, printed `Bridge ACK; UART A 115200 8N1, divisor 1`, and finished:
```
TEST COMPLETE: rx=1024 valid=32 bad=0 detection=32 overrun=0 parity=0 framing=0 breaks=0 I2C_errors=0
```
- **User-run/observed physical bench result:** 32/32 checksum-valid radar detection frames and 0 serial/I2C errors in a single 25-second capture. Actual C4002 presence/motion telemetry is working via CJMCU-752 I2C 0x4D. This supersedes old sensor UART failure. Status reported transitions between PRESENCE and MOTION. Presence distance reports ranged ~1.20–2.17m, motion distance ~0.94–1.62m, speed -0.23 to +0.18m/s, light ~53 lux. These numbers are parsed real sensor reports, *not* user-calibrated measurements; do not mark physical range accuracy as independently user-verified.
- **Candidate sketch used:** `diagnostics/arduino/slot18_c4002_live_readonly/slot18_c4002_live_readonly.ino`; source commit `740bbabb8e0793b20a31412586a6ce321c6bb237`. No display UI was loaded by this serial-only test, which is expected.
- **Next single guided test:** compare real tape-measured target distances at e.g. 1m, 2m and 3m (subject to controlled test environment), record still/moving statuses and actual error, no change to settings/program yet. Check behavior with target absent, as false positives possible. Only after this validate sensor distance / motion. Then separately resume Waveshare touch controller I2C `0x5D` diagnostic and on-device live display UI (not yet implemented). Keep red screen I2C VCC disconnected/insulated; keep power/ground verification.
- Original user factory screen/touch demo previously PASS, but no Slot-18 custom display/touch app user-verified. No test done with LD2450.
- Test status for hardware sensor UART and framed notifications now **USER-RUN PASS**. Distance accuracy, target identification and position still **UNVERIFIED**. No new electrical wiring necessary.

---

# 2026-11-10 — Replacement C4002 external UART link VERIFIED (new user log)

**New evidence:** User uploaded complete Windows Arduino Serial Monitor log (~44,238 lines). Earlier thousands of boot partition errors are historical in this capture; the final section shows the new one-shot C4002 sketch ran through `TEST COMPLETE` after a successful boot. Do NOT treat old boot errors as an ongoing problem when the current test demonstrably executed.

**User-supplied hardware test of `diagnostics/arduino/slot18_c4002_uart_probe/slot18_c4002_uart_probe.ino`:**
- CJMCU SC16IS752 bridge `0x4D` ACK; UART A LCR `0x03`, 115200 baud from verified 1.8432MHz crystal marking / divisor=1.
- Passive 1200ms window: 32 actual UART bytes, beginning `FA F5 AA A5 20 00 00 04 60 01 16 00`; no UART line errors.
- One read-only software version query generated valid 54-byte READ_RESPONSE frame type `0x03`, cmd `0x82`, response code `0x01` (success), checksum PASS. Embedded ASCII version payload `CDJQ_A300_1.0.0_251028@JQ_1.0.0_251028` (surrounded by 0x01 flag and trailing null).
- In same 3000ms response window three 32-byte notification frames type `0x04`, command `0x60`, all checksum PASS. Total **4 valid / 0 invalid / 0 incomplete** framed replies. Line status **0 overrun, 0 parity, 0 framing, 0 break**. 150 response-window bytes.
- **External UART connection physically VERIFIED from user log** (old 2026-10-27 C4002 failure superseded by replacement). Working link through SC16IS752 to real radar, not merely internal loopback. DO NOT conflate this with a fully user-verified radar distance/motion tracking or display UI.
- Manufacturer C4002 source layout suggests notification targetStatus `0x01` (presence) and reported presence distances `0x009E=1.58m`, `0x009B=1.55m`, `0x009C=1.56m`, `0x00A6=1.66m`; sample energy `0x63`. This is **decoded from actual frames, not yet measured against a known-distance target**; status/accuracy under motion must be tested with user-guided physical observation. No XY coordinates.
- **New next candidate:** `diagnostics/arduino/slot18_c4002_live_readonly/slot18_c4002_live_readonly.ino` (commit `740bbabb8e0793b20a31412586a6ce321c6bb237`). Read-only one-shot 25s framed notification capture and human-readable presence/motion/distance/speed output; no radar configuration commands. **NOT compiled, flashed, or user-tested** yet. Next guided test: upload once with previously successful board settings, capture 25s at known static distance and movement; compare measured vs reported; save result, then separately investigate missing touch 0x5D and later LD2450. Keep red screen I2C VCC disconnected/insulated.

---

# 2026-11-10 hardware-test continuation

- Replacement DFRobot C4002 SEN0691 physically installed by user. RUN/OUT indicator activity reported, **not** validated serial data. New unit's full power and signal tests pending.
- Diagnostic candidate added at `diagnostics/arduino/slot18_i2c_discovery/slot18_i2c_discovery.ino`, commit `106cadfda990bc63e0b3beb523e19eef43140807`. Scan `0x4D` bridge / `0x5D` touch first, then return to clock-marking verification and bounded UART response test. User test pending; no C4002 milestone closed.
- Do not connect loose Waveshare red I2C VCC lead to the external 5V rail. Screen remains separately USB powered with shared GND/SDA/SCL. Verify rail polarity before power.

---

# 2026-10-27 real-hardware milestone update (CURRENT)

Prior milestone forecasts below remain conceptual; the user has now **physically received** Waveshare Rev1.2 touchscreen, C4002 SEN0691, LD2450, two CJMCU-752 modules, breadboard/power supplies and oversized enclosure. See `NEXT_CHAT_HANDOFF.md` and `docs/2026-10-27_PHYSICAL_BENCH_CHECKPOINT.md` FIRST.

**Verified / evidence-backed:**
- HW-01 [USER-VERIFIED] Waveshare factory display boots and touch/typing work; factory Wi-Fi/SD self-tests failed, not diagnosed.
- HW-02 [USER-REPORTED PASS] C4002 powered at ~4.9V; no obvious short; LED activity but detection NOT proven.
- HW-03 [TOOL-PROVEN PASS] Arduino compiled/uploaded through CH343 COM3. User backed up original 16MB flash (check hash in handoff). Restore not tested.
- HW-04 [TOOL-PROVEN PASS] ESP32 finds CJMCU-752 at I2C 0x4D via SDA GPIO8 / SCL GPIO9. CJMCU UART channels A/B scratchpad read/write PASS and internal UART loopback PASS.
- HW-05 [BLOCKED] External serial bridge UART A ↔ C4002. Version request had **no validated reply**, 66 garbled bytes / 8 zero bytes from attempted clock divisor choices. Two multimeters read 0.5–1.0V fluctuating at C4002 TX and CJMCU RXA; DMM does not decode UART. Radar TX/RX connectivity, actual oscillator and frame protocol still unverified.

**Next atomic action S18-HW-20261027-01 [PENDING USER RESUME]:** verify CJMCU-752 actual crystal and UART RX idle/logic level and baud divisor, then a *different* bounded serial test with line-status/error counts and checksum verification. Do not repeat identical failed diagnostic without new evidence. Do not change radar configuration/reset as a first response. Continue to parsed C4002 target/distance only after verified physical link, then LD2450 separately. Device display currently serial-only Arduino diagnostic, NOT functional Slot-18 UI.

**Firmware track remains separate:** ESP-IDF v0.2.0 source candidate CI failed at missing `#include "esp_random.h"` (never patched in this chat). Browser prototype automated tests previously passed but UI has no user confirmation.

**Enclosure / power / imaging plans:** current ABS enclosure oversized; future smaller enclosure. Camera overlay/IR/thermal target 50–100ft discussed but hardware not integrated; old Li-ion removable four-cell cartridge details below are HISTORICAL, superseded by user's preference for conventional 18650 cells and a future protected modular power path (electrical solution not chosen). No remote node, Android app or sensor fusion implemented.

---

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


## 2026-10-09 Handoff pause — NO IMPLEMENTATION
- Entire project paused at user's request to stop research loop and save for another chat.
- Read **`NEXT_CHAT_HANDOFF.md`** FIRST for status, context, all pending hardware decisions and exact resume paths.
- No circuit design approved, no board validated, no electrical bench tests, no working firmware v0.2.0 compile (failure at missing esp_random.h declaration), and no user-VERIFIED baseline. Do not re-explain or redo the same board-search comparisons without new primary-source evidence.
- On new instruction: narrow to one question and finish it before additional module research; preserve recoverability. Battery track: manufacturer-led compatibility audit of Waveshare Solar Power Manager (D); firmware track: minimal esp_random.h include fix followed by CI and later real hardware testing.
