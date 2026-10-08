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
