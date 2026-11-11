# Slot-18 C4002 / SEN0691 — Calibration & Sensitivity Controls (PLANNED)

Date: 2026-11-10
Task: S18-C4002-CTRL-20261110-01
Status: **PHASE A USER-VERIFIED PASS (6/6 configuration reads, zero errors)** — original settings captured in `docs/C4002_VERIFIED_CONFIGURATION_2026-11-10.md`. **PHASE B USER-VERIFIED PASS for motion/presence LOW/MID write, readback and restoration** — `diagnostics/arduino/slot18_c4002_sensitivity_control/slot18_c4002_sensitivity_control.ino` (commit `2544503484f8c8f7e8d150561d3e004b5d9e4617`), user-tested on physical radar; HIGH, CUSTOM and behavioral detection effects remain untested. Existing user-verified `diagnostics/arduino/slot18_c4002_continuous/slot18_c4002_continuous.ino` and independently verified `diagnostics/arduino/slot18_c4002_settings_audit/slot18_c4002_settings_audit.ino` remain unchanged.

## Why

User confirms current C4002 live data path works, including >17 minutes of continuous checksum-valid notifications with zero UART/I2C errors. User plans additional controlled tests later. Current environment is cluttered with a wire rack, microwave, TV, DVR, reflective aluminum-faced roof insulation and cats. Presence reported while user was ~20 ft away in another room off-axis. DO NOT attribute detections to the user, cats, walls, ghosts, or false positives without controlled observations. Detection reliability is not the same as target identity or range accuracy.

## DFRobot-supported settings (vendor library)

The C4002 library exposes:
- `startEnvCalibration(delayTime, contTime)`: operator-initiated background/environment capture; DFRobot documentation directs users to vacate the scene. Use a conspicuous delay/countdown and display progress; capture actual calibration notification/response. Never execute on startup.
- `setSensitivity(eMotionDistGate, eLowThreshGroup / eMidThreshGroup / eHighThreshGroup / eCustomThreshGroup)` and the equivalent `ePresenceDistGate`: separate motion/static-presence sensitivity groups. Include current-value query, group selection, and verified result. Do not confuse the groups' threshold directionality with sensor's reported energy.
- `setDetectRange(closest, farthest)`: range bounds in centimeters; vendor states 0-1100 cm, validate `min<=max`.
- `setTargetDisappearDelay(delayTime)`: delay in seconds; avoid interpreting delayed `PRESENCE` as instant false positive.
- `getGateThresh` / `setGateThresh` and `getDistanceGateThresh` / `configureGate`: advanced per-range gate thresholds/enable flags. Expose only after low/medium/high profiles are characterized on hardware.
- `getResolutionMode` and `setResolutionMode`: 80-cm/20-cm gate resolution when supported by installed firmware, after verifying hardware and protocol.
- Light threshold/reporting period may exist; defer until requested or needed.

Manufacturer references:
- https://github.com/DFRobot/DFRobot_C4002
- https://wiki.dfrobot.com/sen0691/
- https://wiki.dfrobot.com/sen0691/docs/23399

## User-facing design target (7-inch Waveshare touchscreen)

1. **Live Monitor** — same verified UART bridge decoder, show genuine C4002 PRESENCE/MOTION/NO_TARGET, separate stale/no-data, and health counts. C4002 is range+speed+presence, **not X/Y track**. Avoid plotting a single location dot.
2. **Calibration** — button `Start Calibration`, preflight text that detection area must be empty (including cats), selectable delay and collection duration with safe defaults after validating manufacturer's protocol. Countdown, in-progress, success/failure based on actual sensor message. Explicit confirmation to write.
3. **Sensitivity** — separate Motion and Presence controls with read-current support and Low/Medium/High. Custom thresholds behind Advanced panel and bounds checking.
4. **Detection Range** — min and max controls in meters (convert to centimeters for vendor protocol), input bounds and cross-field check.
5. **Hold Time** — Target disappear delay in seconds.
6. **Profiles / backup** — capture a **readable snapshot of sensor settings** with time, context and firmware info. Save baseline before any write; `Restore saved profile` only for values whose restore/readback protocol was verified. Show the exact changes and ask before applying. Do not imply snapshot covers every hidden sensor parameter; readback may be partial.
7. **Diagnostics** — track user action, requested write, sensor ACK/response, readback, timeout, and resulting live sensor behavior distinctly; print / export effective configuration and raw diagnostic summaries.
8. **Safety** — never change radar baud, reset firmware, factory reset, or auto-calibrate without separate explicit operator action and recovery instructions; don't change GPIO I2C VCC routing. Bridge initialization is separate from C4002 sensor configuration.

## Incremental implementation and verification

- **Phase A: Read-only audit** — add non-destructive C4002 configuration reads to a separate experimental sketch, verify all vendor responses/checksums via CJMCU-752 and compare against existing firmware. Keep continuously working sketch as fall-back.
- **Phase B: Operator-confirmed low/medium/high motion & presence** — implement one setter at a time with original-value snapshot and readback. Test only when user is ready, with cats excluded where practical and fixed sensor geometry.
- **Phase C: Manual calibration** — only after base values and communication stable, implement delayed calibration and completion-state checks. Record before/after thresholds and restoration method. Explicit user approval for writing sensor settings; no unattended calibration.
- **Phase D: Range/delay/advanced gates** — verify one setting per controlled experiment; implement profiles and restore with feature-specific confirmations.
- **Phase E: Integrate confirmed controls with Waveshare GUI** — follow official display driver/touch bringup first, then sensor panels. Touchscreen I2C addr 0x5D scan still unresolved and is independent of radar UART PASS.

## Current next action

**Next user test, when convenient:** Verify/upload the separate read-only audit sketch. It sends six one-shot manufacturer READ requests via the existing SC16IS752 UART-A adapter: software version (0x82), motion and presence threshold group (two 0x87 reads with gate types 0/1), detection range (0x86), disappearance delay (0x84), and range resolution (0x66). Each has a bounded 2.5s response wait and validates checksum, frame length, type, response code and sensitivity gate before displaying values. The sketch does not persist a snapshot; capture the entire Serial Monitor output for baseline. Check that each query returns a successful response before implementing writes. After testing, the original continuous monitor can be reuploaded. The DFRobot native UART Arduino library cannot directly communicate through SC16IS752 I2C; the diagnostic uses the proven manual bridge adapter. Do not auto-calibrate or send sensor-setting writes until original values are read and verified.

Recommended coding setting: Medium effort for individual UART bridge read-only commands, escalate High only if protocol replies/bridge I2C prove unexpectedly complex.

## 2026-11-10 progress: Phase A verified; Phase B optional candidate

The user ran the six-query audit and obtained 6/6 successful replies with zero timeouts, UART errors, I2C errors or invalid frames. Actual values are saved in `docs/C4002_VERIFIED_CONFIGURATION_2026-11-10.md`: both sensitivity groups MID, range 0–1100cm, hold time 15s, resolution 80cm, firmware `CDJQ_A300_1.0.0_251028@JQ_1.0.0_251028`.

Next experimental sketch: `diagnostics/arduino/slot18_c4002_sensitivity_control/slot18_c4002_sensitivity_control.ino` is **candidate only** and needs Arduino IDE compile + physical UART testing before any claims. On boot it reads both sensitivity presets, no C4002 writes. `STATUS` is repeatable read-only; `PLAN MOTION LOW|MID|HIGH` or `PLAN PRESENCE LOW|MID|HIGH` arms a proposed change for up to 60s. `CONFIRM` must be typed separately to write a single group, then fetches immediate sensor readback. `CANCEL` aborts. There is no automatic rollback or guarantee that values reset on power loss. User can explicitly restore original MID individually. Range/hold-time/environmental calibration writes remain unavailable in this candidate.

First test should be **read-only**: compile, flash on same board, open UART USB-C CH343 Serial Monitor at 115200, view startup status and send `STATUS` using Newline; verify both current groups MID, zero I2C errors. If that fails, do not attempt `PLAN` or `CONFIRM`. No wiring or power changes needed.

## 2026-11-10 closeout: Phase B completed and baseline restored

The user successfully tested `PLAN MOTION LOW` / `CONFIRM` / readback `LOW`, restoration `PLAN MOTION MID` / `CONFIRM` / readback `MID`, then separately the same MID -> LOW -> MID cycle for PRESENCE. Final manual `STATUS` in user-uploaded `Pasted text(20261111-024612).txt` showed **Motion MID 1; Presence MID 1; valid_frames=575; bad=0; notices=551; UART_errors=0; I2C_errors=0**. Verified control writes and configuration changes, not measured real-world detection effectiveness.

**Next milestone**: select single new bounded capability — read-before/write/readback/restore of distance range (from baseline 0–1100cm) first, then target disappearance delay (baseline 15s), as separate operator-confirmed steps. No calibration until unoccupied environment and sensor confirmation are available. Or switch to physical touchscreen hardware bringup as an independent stream. Keep existing working firmware source files intact.
