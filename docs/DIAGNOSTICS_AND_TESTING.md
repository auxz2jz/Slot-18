# Diagnostics Coverage Map & Guided Tests

Mandatory global rules: https://github.com/auxz2jz/master-instruction-library
Diagnostic standard and guided testing apply to ALL future firmware and UI features.

## Current browser prototype features + coverage map
| Feature | Action/Trigger | Request | Verification | Failure | Logged categories | Guided step |
|---|---|---|---|---|---|---|
| Simulated radar | startup | create sample reports | valid track and presence events | invalid sample/canvas error | OPERATION_START/RESULT/ERROR | T1 |
| Direction select | user selects orientation | update heading | heading label and transformed map position | invalid heading | USER_ACTION/STATE_TRANSITION/OPERATION_RESULT | T2 |
| Pause/resume | user button | change simulation status | text changes, render loop state | unexpected state | USER_ACTION/STATE_TRANSITION/OPERATION_RESULT | T3 |
| Diagnostic export | user button | serialize buffered trace | JSONL blob download produced | serialization/createObjectURL error | USER_ACTION/OPERATION_START/OPERATION_RESULT/ERROR | T4 |
| Guided tests | user button and PASS/FAIL | record visual judgment | test report with step, status | no test or save error | USER_ACTION/TEST_RESULT/ERROR | T1-T4 |

## Event schema
Each trace event should include session_id, event_id or monotonic sequence, timestamp_utc, monotonic elapsed_ms, candidate_version, category, operation_id when applicable, component, result or error. Traces must separate user intent from operation result; simulated detection itself is never proof a physical radar works.
Browser proof-of-concept uses bounded rolling localStorage logs when allowed; otherwise in-memory logs, export available in both cases. User may clear browser storage. No raw audio/video, identity or real GPS in diagnostic exports at this stage.

## "Test This Version" manual protocol (v0.1.0 candidate)
- T1: Open prototype/index.html in browser. Confirm prominent SIMULATED ONLY label, radar circles and movement dot; make no assumption of live sensor.
- T2: Change sensor facing North/East/South/West. Verify direction label follows control and target plot rotates. Inspect visual alignment manually.
- T3: Pause and resume. Verify control text and status, not sensor activity.
- T4: Export JSONL diagnostics. Check download contains USER_ACTION and OPERATION_RESULT events with timestamps and session.
Use on-screen Test This Version panel to mark each manually PASS/FAIL. Test progress remains browser-local when storage enabled; export test results when needed. No hardware tests until hardware is in hand.

## Future firmware guided tests
- Screen backlight and touch responsiveness; UI navigation action/result, FPS/memory checks, log export.
- Each radar: initial handshake, model, frames/s, CRC/framing validity, disconnected/reconnected, X/Y and range vs measured tape-distance, orientation.
- C4002 static person: standing/sitting and lying relatively still; record detected/not detected including misses and non-human false detections.
- Concurrent three radars: compare sequential vs simultaneous operation, rate/jitter/noise; interference.
- Remote network: register 1 node then five, link loss/rejoin, stale readings, GNSS accuracy circle, pose/calibration.
- Battery: measured current idle/display/backlight/radars/network, undervoltage, reboot and auto-recover.
- Enclosure and pole: near-hand/body occlusion, multipath, moving mode and vertical coverage.
Persist diagnostic sessions, correlation IDs, crash/watchdog evidence, per-feature test results and user confirmation. Never label a compile-only test as physical VERIFIED.


## v0.2.0 ESP-IDF hardware screen candidate — actual features and tests
**Source:** firmware/esp-idf/main/main.c . Only simulated data, no real radar input.
| Feature | Trigger | Request | State/result verification | Failure signal | Events | Test |
|---|---|---|---|---|---|---|
| Screen bring-up | boot | init RGB LCD / GT911 / LVGL | visible banner and touch reaction **user-confirmed** | ESP_ERROR_CHECK abort / serial panic / blank screen | OPERATION_START, OPERATION_RESULT; IDF panic | H1 |
| Simulated target | LVGL timer | calculate fake local XY | moving dot and range label agree, independent of physical objects | graphical mismatch or frozen task | source=simulation, user observation | H2 |
| Heading | Facing button | rotate among N/E/S/W | label and dot rotation | label wrong/dot doesn't turn | USER_ACTION, OPERATION_RESULT | H3 |
| Pause | Pause/Resume button | stop/resume sim time | dot stops then starts | dot moves when paused | USER_ACTION, STATE_TRANSITION, OPERATION_RESULT | H4 |
| Guided test | Test, PASS, FAIL | activate/rate each manual check | test label advances, user rating recorded | wrong sequence/premature success | USER_ACTION, TEST_RESULT | H5 |
| Diagnostics | LOGS button | print recent NVS ring to serial | BEGIN/END and JSON log messages appear in PC monitor | no persistent NVS / UART not wired | USER_ACTION, OPERATION_RESULT, ERROR and ESP log | H6 |

### Human guided steps when hardware is available
- H1: Flash unmodified official Waveshare demo first; check touch and display. Record reference success/failure. Then flash Slot-18 and confirm amber SIMULATED ONLY header and user-observed screen.
- H2: Confirm the green XY demo dot moves automatically. Covering radar (if physically present but not electrically connected) should NOT affect this simulation.
- H3: Touch Facing four times. Confirm N/E/S/W label cycles, and dot direction rotates around center without claiming world compass integration.
- H4: Touch Pause; dot position should stop changing. Touch Resume; dot should move again.
- H5: Touch Test. It displays three prompts. At each step choose PASS or FAIL based on what actually happened. Check completion summary.
- H6: Attach programming USB to computer / UART log connection per Waveshare; press LOGS and confirm export markers and valid structured messages. Confirm timestamps use boot uptime, not absolute time.
- Mark user verification only after H1-H6 are physically checked and reported; a successful GitHub compile is not physical validation.

### Observability limitations in this candidate
- The ESP-IDF panic/abort handler and serial console are still needed for crashes before UI startup; no automatic export of reboot crash dumps yet.
- NVS event ring stores the last 32 events, user-action events cause flash writes; later we can move high-frequency telemetry to a non-wearing queue and export on demand.
- Without RTC/network time, diagnostic timestamps are monotonic elapsed milliseconds and session IDs; do not claim UTC.
- Source data is always simulator in v0.2.0. No actual LD2450 frame checking, CRC, UART pin assignment or capture exists yet.

## C4002 continuous bridge live test — S18-DIAG-20261110-CONTINUOUS-01 (candidate; 2026-11-10)

- **Firmware:** `diagnostics/arduino/slot18_c4002_continuous/slot18_c4002_continuous.ino`, commit `af2e3af25f20d0fa45afe71a04f28b15432188c8`. Separate from user-verified bounded 25-second original. Not compiled or user-tested yet.
- **Trigger:** Upload using previously verified Waveshare board/16MB partition/PSRAM Enabled/COM3 settings. Open Serial Monitor 115200, reset once if opening missed startup.
- **Wire baseline:** CJMCU 0x4D on GPIO8 SDA / GPIO9 SCL, 1.8432MHz bridge crystal, UART A divisor 1 for 115200 8N1, shared ground and correctly verified 5V breadboard supply; red Waveshare I2C VCC lead disconnected and insulated.
- **Output:** Live checksum-checked type 0x04 cmd 0x60 notification reports, range/status/speed, NO_TARGET (radar observation, no stale range), NO NEW DATA (5-second communication stale warning, distinct from NO_TARGET), 30s SUMMARY counters of received bytes, checksum failures, line and I2C errors, last detection packet age.
- **Guided check 1 (UART/continuous):** Confirm bridge ACK and stream continues *beyond 60 seconds*, at least two 30-second SUMMARY lines with valid detection counts increasing and no ongoing errors. Mark success only from observed data, not upload success.
- **Guided check 2 (motion):** At stationary radar, sit nearby and move hand toward/away; record status/distance changes. Target association is **unknown**, so do not conclude hand tracking.
- **Guided check 3 (absence):** Vacate detection area and wait 20–30 seconds or longer according to actual radar configured delay. Record NO_TARGET if observed. If still PRESENCE, note surroundings/reflections and do not automatically report failure. A NO NEW DATA message is transport loss, NOT successful no-target detection.
- **Guided check 4 (accuracy, later):** Compare radar's distance at physically measured positions (e.g., 1,2,3m); keep presence distance separate from motion distance.
- **Stop:** Close Serial Monitor to stop local display; firmware still runs while powered. Remove power or upload a different sketch to stop firmware. No display or touch tests implied.
- **Evidence:** Save a concise log slice including startup, several detections, two SUMMARY reports, and absence outcome. Label user-verified only after physical test.

## Phase A read-only C4002 settings audit — S18-C4002-AUDIT-20261110-01

- Source: `diagnostics/arduino/slot18_c4002_settings_audit/slot18_c4002_settings_audit.ino`, commit `5fb1cfb371197354672731199ad7db2c28c12366`. **Candidate only, not compiled or hardware tested.** Existing user-verified continuous monitor remains unchanged.
- Hardware preconditions: same successful Waveshare ESP32-S3 Touch LCD 7 + CJMCU-752 0x4D + C4002 radar wiring; red Waveshare I2C 5V wire disconnected/insulated; don't rewire powered components.
- Run: open raw audit source, replace entire Arduino editor, select known-good 16 MB board/partition/PSRAM/COM3, Verify then Upload. Serial Monitor 115200; press RESET once if necessary. This runs once per reset then stops.
- Expected: boot `SLOT-18 C4002 READ-ONLY SETTINGS AUDIT`, `Bridge ACK`; six ordered `QUERY x/6` blocks (version, motion, presence, detection range, disappearance delay, resolution), then `AUDIT COMPLETE`. Responses should have sensor status=1 and nonzero appropriate payload; checksum and matching cmd/gate verified before PASS. Partial/timeouts are **not** PASS. Zero UART/I2C error flags in stationary powered condition; record actual exceptions rather than assuming failures of hardware.
- Evidence: paste all six query blocks (including raw payload), summary counts, and test circumstances. Do not claim values are user-verified before log. Read requests do not change radar settings; script has no automatic retry, no sensor calibration or factory reset.
- On success, capture full output as baseline configuration before any future sensitivity write. To return to real-time observations re-upload separate user-verified continuous sketch.
