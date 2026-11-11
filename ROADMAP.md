# 2026-11-10 — C4002 DISAPPEARANCE DELAY RESTORED TO 15s (FULL USER-VERIFIED PASS)

- User pasted direct hardware Serial Monitor evidence for delay-control `diagnostics/arduino/slot18_c4002_delay_control/slot18_c4002_delay_control.ino`, proving the **full 15s -> 5s -> 15s** round trip via separate explicit `PLAN DELAY` and `CONFIRM` actions:
```
CURRENT DELAY: 5 seconds
PROPOSED DELAY: 5 -> 15 seconds
Type CONFIRM within 60 seconds for one setting change.
Or CANCEL to leave delay unchanged.
APPLYING DELAY 5 -> 15 seconds (one radar WRITE)...
WRITE ACK: radar reported success.
READBACK DELAY: 15 seconds
VERIFIED: intended disappearance delay is active.
Original delay: 15 seconds.

STATUS: current C4002 target disappearance delay:
  Disappearance delay=15 seconds
  Link diagnostics: valid=163 bad=0 notices=152 UART_errors=0 I2C_errors=0
  Original verified delay=15 seconds.
  STATUS is READ ONLY; no settings changed.
```
- **Hardware-verified PASS:** target disappearance delay cmd 0x84 one-write success ACK/readback for 15->5 and 5->15; final separate STATUS confirms restored 15 seconds and zero corrupt frames, UART errors, I2C errors. Prior note `restoration pending` is **SUPERSEDED**.
- Four major configurable controls now have complete change-and-restore tests, all back at verified original values:
  - Motion sensitivity MID(1) -> LOW(0) -> MID(1)
  - Presence sensitivity MID(1) -> LOW(0) -> MID(1)
  - Detection range 0..1100 cm -> 0..500 cm -> 0..1100 cm
  - Target disappearance delay 15 s -> 5 s -> 15 s
- Other read-only audited baseline: 80 cm range gate resolution (code 0), original software version `CDJQ_A300_1.0.0_251028@JQ_1.0.0_251028`. No user-test for changing range resolution or custom individual distance gates. No environmental calibration performed.
- **Important scope:** Verified configuration writes and independent reads, not controlled human detection, real-world sensor sensitivity curves, radial motion accuracy, detection range cutoff, true 5/15s timing, XY position or through-wall ability. Prior continuous monitoring had stable 17m run with no errors but no identified targets.
- All five separate known-good sketches/experimental demos preserved in repo: continuous, read-only audit, sensitivity control, range control, and delay control. No coding change required to mark delay control verified. Next useful step: plan read-only resolution/status control or integrate current proven settings into physical 7-inch Waveshare GUI; choose one atomic task. Respect user request to defer power discussion.

---

# 2026-11-10 — USER-VERIFIED DELAY WRITE 15s -> 5s, RESTORATION PENDING

The user pasted direct Serial Monitor evidence after initial delay STATUS on the new source `diagnostics/arduino/slot18_c4002_delay_control/slot18_c4002_delay_control.ino`:
```
CURRENT DELAY: 15 seconds
PROPOSED DELAY: 15 -> 5 seconds
Type CONFIRM within 60 seconds for one setting change.
Or CANCEL to leave delay unchanged.
APPLYING DELAY 15 -> 5 seconds (one radar WRITE)...
WRITE ACK: radar reported success.
READBACK DELAY: 5 seconds
VERIFIED: intended disappearance delay is active.
Original delay: 15 seconds.
Restore with PLAN DELAY 15, then CONFIRM.
```
- **USER-VERIFIED PASS** for operator-confirmed C4002 delay write 0x84 (15->5 sec), sensor success ACK and independent checksum-valid readback. **LAST VERIFIED CURRENT DELAY IS 5 SECONDS**, NOT restored yet. Earlier STATUS in same capture was 15 sec before the write, with `valid=14 bad=0 notices=12 UART_errors=0 I2C_errors=0`.
- This verifies **configuration update**, not independently measured physical hold-time effectiveness, and no claim of exact five seconds measured with stopwatch.
- **IMMEDIATE NEXT single user test:** with same program running, send `PLAN DELAY 15`, verify it says `CURRENT DELAY: 5 seconds` and `PROPOSED DELAY: 5 -> 15 seconds`, then `CONFIRM` within 60 sec, then `STATUS`; share output to verify 15sec restoration. If write/ACK/readback fails do not claim original restored; troubleshoot.
- Original baseline: motion MID 1, presence MID 1, range min0/max1100cm, disappearance delay 15sec, resolution code0 80cm. Motion+presence and range cycle previously user-verified restored; latest delay presently at 5sec and restoration pending.
- Keep all known-good sketches intact; no rewiring or power discussion. User prefers one atomic test each turn.

---

# 2026-11-10 — Delay control startup/manual STATUS USER-VERIFIED PASS; no write yet

User provided live output from `diagnostics/arduino/slot18_c4002_delay_control/slot18_c4002_delay_control.ino`:
```
STATUS: current C4002 target disappearance delay:
  Disappearance delay=15 seconds
  Link diagnostics: valid=14 bad=0 notices=12 UART_errors=0 I2C_errors=0
  Original verified delay=15 seconds.
  STATUS is READ ONLY; no settings changed.
```
- **USER-VERIFIED**: C4002 disappearance-delay read via vendor cmd0x84, serial command processing, 14 checksum-valid frames (12 notifications), zero bad frames/UART or I2C errors. The pilot compiles and runs on Waveshare ESP32-S3 via CJMCU bridge; read-only operation passed. The 15-second original setting is unchanged. **Write/readback untested yet**.
- **Next single controlled action:** user sends `PLAN DELAY 5`, verifies `CURRENT DELAY: 15 seconds` and `PROPOSED DELAY: 15 -> 5 seconds`, then sends `CONFIRM` within 60 seconds and shares resulting write ACK and `READBACK DELAY`. If any mismatch, cancel and do not write. If successful, return to original later via `PLAN DELAY 15`/`CONFIRM`, followed by `STATUS`. Never claim restoration before seeing output.
- Other last verified settings remain Motion MID1, Presence MID1, range0..1100cm; 80cm gate resolution from earlier audit, with no further changes. No source changes necessary. Preserve known-good continuous, audit, sensitivity and range control sketches. User prefers one atomic test and has deferred power discussion.

---

# 2026-11-10 — Separate C4002 disappearance-delay control CANDIDATE saved (untested)

- After the **user-verified** full range 0–1100cm -> 0–500cm -> 0–1100cm round-trip, added a separate read-first/guarded-one-write/readback candidate:
  `diagnostics/arduino/slot18_c4002_delay_control/slot18_c4002_delay_control.ino` at commit `bf4118c35341f6ce15b6673a6eb1c647ae46d84f`.
- Vendor library `DFRobot/DFRobot_C4002` checked: `setTargetDisappearDelay(uint16_t)` uses cmd **0x84**, write frame request0x00, dataLen **6** with 2-byte little-endian seconds; `getTargetDisappearDelay()` read request0x01 dataLen **4** and returned two-byte seconds. Documented manufacturer valid field 0..65535s, experimental control intentionally accepts only 0..60s to avoid accidental long holds. Original user-read 15s.
- Separate sketch reuses proven Waveshare ESP32-S3 GPIO8/GPIO9, SC16IS752 bridge I2C0x4D, UART A 115200, CH343 USB UART. Startup/`STATUS` READ ONLY. Commands `STATUS`, `PLAN DELAY 5`, `PLAN DELAY 15` for restoration, `CONFIRM` (60s expiry, rereads and checks before single write, readback after), `CANCEL`, `HELP`. Does not touch range, sensitivities, resolution, environment calibration or firmware baud.
- Source was structurally reviewed (balanced braces, expected manufacturer request frame fields) but **not Arduino compiled, uploaded or hardware tested yet**. Do not claim delay reads/writes verified for this new control. All four previously user-tested source files remain preserved in separate locations.
- **NEXT ATOMIC USER ACTION:** open new `slot18_c4002_delay_control.ino` in Arduino IDE, compile/upload using original Waveshare CH343 UART USB-C and wiring, open Serial Monitor at **115200 / Newline**, send `STATUS`. Expect `Disappearance delay=15 seconds` and zero I2C/UART/bad frames. Do **not** ask user to write `PLAN DELAY 5` / `CONFIRM` until initial read-only test user-verified. Then after single confirmed 5-second change and verified readback, separate restore with `PLAN DELAY 15` / `CONFIRM`, final STATUS.
- Range sensor currently last-verified baseline 0..1100cm; motion MID1, presence MID1. Performance/calibration not tested; hold setpoint modification does not prove physical clearance timing until a separate field test.

---

# 2026-11-10 — C4002 RANGE CONTROL COMPLETE, ORIGINAL 0–1100cm RANGE RESTORED

User pasted direct ESP32-S3 Arduino Serial Monitor evidence (after previous successful `PLAN RANGE 0 500` / `CONFIRM`) showing restored range:
```
CURRENT RANGE: 0..500 cm
PROPOSED RANGE: 0..500 -> 0..1100 cm
Type CONFIRM within 60 seconds to change this setting.
Or CANCEL to leave current range unchanged.
APPLYING RANGE 0..500 -> 0..1100 cm (one radar WRITE)...
WRITE ACK: radar reported success.
READBACK RANGE: 0..1100 cm
VERIFIED: intended detection range is active.

STATUS: current C4002 radar detection range:
  Range min=0 cm (0.00 m), max=1100 cm (11.00 m)
  Link diagnostics: valid=158 bad=0 notices=147 UART_errors=0 I2C_errors=0
  Original verified range=0..1100 cm.
  STATUS is READ ONLY; no settings changed.
```
- **RANGE CONTROL USER-VERIFIED COMPLETE**: original 0..1100cm -> experimental 0..500cm -> restored 0..1100cm, each with write ACK and independent readback; final manual STATUS confirms restored value. Last checkpoint's "restoration pending" is **SUPERSEDED**.
- The original configurations are restored for Motion MID1, Presence MID1, Range min 0cm max 1100cm. Disappearance delay last read 15s, resolution last read code0 80cm. No change to those latter settings made in any experiment, but they were not re-read in this range session.
- Physical detection accuracy/cutoff not measured, just sensor accepted configuration.
- Next anticipated separate bounded milestone: user-confirmed control for **target disappearance delay** (DFRobot cmd0x84, baseline15s), with safe read-only initial test, single confirmed change, readback, and explicit restoration to 15s; preserve all user-verified Arduino sketches untouched.

---

# 2026-11-10 — USER-VERIFIED C4002 RANGE WRITE 0–1100cm -> 0–500cm; RESTORATION PENDING

User pasted full serial log demonstrating successful explicit operator-confirmed first RANGE WRITE using `diagnostics/arduino/slot18_c4002_range_control/slot18_c4002_range_control.ino`. Relevant new lines:
```
CURRENT RANGE: 0..1100 cm
PROPOSED RANGE: 0..1100 -> 0..500 cm
Type CONFIRM within 60 seconds to change this setting.
Or CANCEL to leave current range unchanged.
APPLYING RANGE 0..1100 -> 0..500 cm (one radar WRITE)...
WRITE ACK: radar reported success.
READBACK RANGE: 0..500 cm
VERIFIED: intended detection range is active.
Original hardware range is 0..1100 cm.
To restore: PLAN RANGE 0 1100, then CONFIRM.
```
- **Physical hardware status: USER-VERIFIED PASS** for paired minimum/maximum detection-range WRITE (DFRobot cmd 0x86) and independent successful readback. Current *last verified* radar detection range: **0–500cm (0–5m)**. This supersedes prior note saying RANGE WRITE untested. **Restoration has NOT YET been verified**. Other original settings: Motion MID 1, Presence MID 1, delay 15 seconds, 80cm gates (previously independently verified; not re-read in this latest session).
- **IMMEDIATE NEXT guided test**: with same working sketch and UART USB-C CH343 Serial Monitor 115200 / Newline, send `PLAN RANGE 0 1100`, check proposes `0..500 -> 0..1100 cm`, send `CONFIRM` within 60s, then `STATUS`; expect `READBACK RANGE: 0..1100 cm` and final status 0–1100cm. Capture result. If unexpected output, do NOT assume restored.
- Verified firmware behavior is configuration write/readback, **not** measured distance detection performance or target identity. No hardware wiring/source changes needed. Existing working continuous, audit and sensitivity control sketches preserved.
- Continue respecting user's one-step test preference; no discussion of power supply until requested.

---

# 2026-11-10 — RANGE CONTROL READ-ONLY BOOT AND MANUAL STATUS: USER-VERIFIED PASS

User shared exact Serial Monitor output for the separate range-control firmware `diagnostics/arduino/slot18_c4002_range_control/slot18_c4002_range_control.ino`:
```
SLOT-18 C4002 RANGE CONTROL — CONFIRM REQUIRED
No automatic C4002 writes. STATUS is read-only.
Bridge OK; UART A 115200, 8N1.

STATUS: current C4002 radar detection range:
  Range min=0 cm (0.00 m), max=1100 cm (11.00 m)
  Link diagnostics: valid=1 bad=0 notices=0 UART_errors=0 I2C_errors=0
  Original verified range=0..1100 cm.
  STATUS is READ ONLY; no settings changed.

COMMANDS (Serial Monitor 115200, Newline):
  STATUS
  PLAN RANGE <min_cm> <max_cm>   e.g. PLAN RANGE 0 500
  PLAN RANGE 0 1100             restore original after testing
  CONFIRM                       apply one pending range write
  CANCEL                        discard planned change
  HELP
PLAN never writes. CONFIRM expires 60s after PLAN.

STATUS: current C4002 radar detection range:
  Range min=0 cm (0.00 m), max=1100 cm (11.00 m)
  Link diagnostics: valid=20 bad=0 notices=18 UART_errors=0 I2C_errors=0
  Original verified range=0..1100 cm.
  STATUS is READ ONLY; no settings changed.
```
- **Status PASS** for startup and manual serial `STATUS`, with both range READ requests returning 0..1100cm; zero invalid frames, zero UART or I2C errors. This user-tested candidate **compiles, uploads, and can read the sensor's range**. Do not claim its range WRITE/readback functionality has been tested yet; sensor stays original 0–1100cm.
- **NEXT controlled test** using running sketch: send `PLAN RANGE 0 500`, ensure it reports `CURRENT RANGE: 0..1100 cm` and `PROPOSED RANGE: 0..1100 -> 0..500 cm`, then `CONFIRM` within 60s. Expect write ACK and `READBACK RANGE: 0..500 cm`, verify and share output. **After that**, restore original with `PLAN RANGE 0 1100` / `CONFIRM` and final `STATUS`, separately and only after successful range change confirmation. If any mismatch, `CANCEL` and do not write.
- Independently verified sensitivity group motion MID 1/presence MID 1, disappearance delay 15s and gate resolution 80cm from previous tests. Preserve known-good sensitivity control, settings-audit and continuous monitor sketches; no other source changes. User prefers one experiment at a time. Power discussion deferred.

---

# 2026-11-10 — RANGE CONTROL CANDIDATE ADDED; ABS enclosure clearance guidance

- User explicitly authorized next **adjustable detection range** development (not a hardware write) after both sensitivity groups restored to MID. Created **separate experimental read-before/confirmed-write/readback range control Arduino sketch**:
  `diagnostics/arduino/slot18_c4002_range_control/slot18_c4002_range_control.ino`, GitHub commit `196e2095e67c9bb2483509f8493d0e7663e52c7c`.
- Firmware source designed for exact previously proven Waveshare ESP32-S3 Touch LCD 7 Rev1.2 + CJMCU-752 I2C 0x4D + C4002 115200UART A; unchanged GPIO8 SDA, GPIO9 SCL and bridge divider. Original continuous, settings-audit and sensitivity-control sketches remain untouched.
- Protocol verified against DFRobot library `DFRobot/DFRobot_C4002/src/DFRobot_C4002.cpp`: vendor setDetectRange uses **command 0x86**, type0 WRITE, data length **8** including 4-byte min/max little-endian payload; getDetectRange uses cmd 0x86, type1 READ, data length 4 (no payload); expected READ reply contains four bytes min/max.
- Candidate startup/STATUS is READ-ONLY; supports `STATUS`, `PLAN RANGE <min_cm> <max_cm>` (0<=min<max<=1100), `CONFIRM` within 60 seconds, `CANCEL`, `HELP`. `PLAN RANGE 0 1100` is explicit restoration path. CONFIRM re-reads old value and guards it before writing exactly one min/max range update, then independently reads back. No calibration, reset, baud, resolution, sensitivity or disappearance-delay modifications. **NO RANGE WRITE HAS YET BEEN TESTED OR EXECUTED**. Source was structurally checked (balanced braces etc) but **Arduino compile and real physical upload/hardware are NOT yet verified**; an attempted independent g++ C++ syntax check could not fetch raw Github content in a network-restricted container; do not represent that as passed. Vendor code comparison was performed by Github tool.
- Original restored sensor baseline: motion MID (1), presence MID (1), detection range 0–1100cm, disappearance hold 15s, 80cm resolution; stored `docs/C4002_VERIFIED_CONFIGURATION_2026-11-10.md`. Last successful user-run sensitivity STATUS 575 valid frames, 0 UART/I2C errors.
- Recommended **FIRST single read-only test**: User downloads/opens new range-control .ino from repo, compiles/uploads via Arduino IDE while USB cable plugged into Waveshare UART USB-C CH343 port, opens Serial Monitor baud 115200 and Newline, presses RESET if startup missed, types `STATUS` and copies fresh results. Expect `Range min=0 cm (0.00 m), max=1100 cm (11.00 m)`. STOP before `PLAN`/`CONFIRM` until this is confirmed. Only after that, one separate test `PLAN RANGE 0 500` followed by `CONFIRM` and readback, then restore `PLAN RANGE 0 1100`/CONFIRM independently.
- **Enclosure guidance:** DFRobot maker community 24GHz enclosure design note (July 2026, https://community.dfrobot.com/makelog-318788.html) recommends antenna-to-enclosure-cover **minimum 6.2mm** (greater than 6.2mm ideally), PC/ABS wall thickness ~3–4mm, uniform flat unmetalized front panel, no metal paint/carbon particles, vibration avoided, verify by testing. User has standoffs 5, 6, 8, 10, 12, 15, 18, 20, 25mm. Recommend **8mm target clearance, or 10mm** between antenna face (gold pads) and inside of ABS FRONT COVER; these are NOT necessarily identical to rear mounting standoff length, which depends on case depth and PCB thickness. 5/6mm face clearance too close for this guideline. Cover may block on-board ambient light.
- "Other things on the screen": previous Serial Monitor live telemetry fields PRESENCE/MOTION, presence/motion distance, signal energy, radial speed, direction, ambient light lux, countdown, distance gates; NOT on-screen target X/Y, human identification, or proven through-wall detection. Native Waveshare touchscreen visual UI not integrated in these diagnostics yet.
- Power supply topic remains deferred per user preference.

---

# 2026-11-10 — PHASE B COMPLETE: PRESENCE LOW -> MID RESTORED, BOTH GROUPS AT ORIGINAL MID (USER VERIFIED)

- Latest user-uploaded Serial Monitor `Pasted text(20261111-024612).txt`, 244 lines cumulative; **new append lines 227–243**:
```
CURRENT PRESENCE: LOW (0)
PROPOSED PRESENCE: LOW -> MID
Type CONFIRM within 60s to change this ONE setting.
Or CANCEL to leave everything unchanged.
APPLYING PRESENCE LOW -> MID (single radar WRITE)...
WRITE ACK: radar reported success.
READBACK PRESENCE: MID (1)
VERIFIED: intended setting is active.

STATUS: current radar sensitivity groups:
  Motion  = MID (group 1)
  Presence= MID (group 1)
  Link diagnostics: valid_frames=575 bad=0 notices=551 UART_errors=0 I2C_errors=0
  Original hardware snapshot: MOTION MID, PRESENCE MID.
  NO sensor settings changed by STATUS.
```
- **User-verified PASS:** presence setting restored via explicit `PLAN PRESENCE MID` and `CONFIRM`; vendor acknowledged write, separate readback returned MID, and subsequent independent STATUS confirmed both MOTION MID 1 and PRESENCE MID 1. No UART/I2C/packet errors in resulting cumulative active-control summary. Prior note that restoration was pending is **SUPERSEDED**.
- **Full phase B status:** C4002 motion sensitivity MID -> LOW -> MID (both writes/readbacks verified); presence sensitivity MID -> LOW -> MID (both writes/readbacks verified). Interactive serial STATUS and initial 6/6 read-only settings audit user verified. Serial-monitor operator-confirmed candidate `diagnostics/arduino/slot18_c4002_sensitivity_control/slot18_c4002_sensitivity_control.ino` is USER-VERIFIED for LOW/MID write and restore on both gates. HIGH/Custom and effectiveness in detecting different targets remain **untested**.
- **Actual original configuration at last verification:** motion MID 1, presence MID 1. Earlier audit separately established 0–1100cm range, 15-second disappearance hold, 80cm resolution and firmware `CDJQ_A300_1.0.0_251028@JQ_1.0.0_251028`, but these latter fields were not re-read in latest session. Saved human-readable original baseline: `docs/C4002_VERIFIED_CONFIGURATION_2026-11-10.md`.
- **Recommended next development**: leave hardware unchanged; implement a separate **operator-confirmed range/delay command sketch** with verified baseline read-before, single setting write, immediate readback and original restoration support, or begin actual Waveshare touchscreen hardware UI bring-up if user prefers. Vendor range/delay read commands are already proven, writes not yet. Manual environment calibration should be deferred until user can isolate the test area from clutter/cats, with an unoccupied sensing area and an explicit confirmation flow. Respect user's preference for small atomic tests and no power tangent.
- Preserve user-verified continuous monitor, audit and sensitivity control sources; no unrelated changes.

---

# 2026-11-10 — USER-VERIFIED PRESENCE SENSITIVITY MID -> LOW WRITE + READBACK; RESTORATION PENDING

- Latest user uploaded `Pasted text(20261111-024500).txt` (227 lines), which includes earlier previous sessions in same cumulative Serial Monitor. The **new append** lines 217–226 from active sensitivity control:
```
CURRENT PRESENCE: MID (1)
PROPOSED PRESENCE: MID -> LOW
Type CONFIRM within 60s to change this ONE setting.
Or CANCEL to leave everything unchanged.
APPLYING PRESENCE MID -> LOW (single radar WRITE)...
WRITE ACK: radar reported success.
READBACK PRESENCE: LOW (0)
VERIFIED: intended setting is active.
Original hardware baseline is MID for each group.
To restore, PLAN this group MID, then CONFIRM separately.
```
- **Result: USER-VERIFIED** bidirectional presence threshold group 0x87 write ACK and readback; presence **LOW group 0 is currently last verified**. Motion **MID 1** was last independently verified following prior LOW->MID restoration. Do NOT claim presence is restored to MID yet.
- Active source: `diagnostics/arduino/slot18_c4002_sensitivity_control/slot18_c4002_sensitivity_control.ino`; no source changes needed.
- **IMMEDIATE NEXT USER TEST:** With Serial Monitor 115200 and Newline, send `PLAN PRESENCE MID`. Verify it says `CURRENT PRESENCE: LOW (0)` and `PROPOSED PRESENCE: LOW -> MID`. Send `CONFIRM` within 60s, then `STATUS`. Capture output proving presence MID (1) and motion MID (1), independent readback, no errors. Any unexpected response: do not assume restoration; troubleshoot before continuing.
- This proves *programmatic config preset control* for motion and presence, not behavior/effective detection sensitivity, physical calibration, or person identification. Range remains 0–1100cm, hold 15s, resolution 80cm as baseline prior (not newly reaudited).
- Current user-tested motion toggle: MID -> LOW -> MID success. Current user-tested presence toggle: MID -> LOW success; return to MID pending.
- Original baseline backup: `docs/C4002_VERIFIED_CONFIGURATION_2026-11-10.md`. Existing read-only audit and continuous monitor untouched. Power topic still deferred.

---

# 2026-11-10 — USER-VERIFIED C4002 MOTION SENSITIVITY RESTORATION COMPLETE

- User uploaded Serial Monitor `Pasted text(20261111-024332).txt` (217 lines). Latest appended lines 200–216 show `PLAN MOTION MID` initiated with `CURRENT MOTION: LOW (0)`, `PROPOSED MOTION: LOW -> MID`, then separately `CONFIRM` accepted and `WRITE ACK: radar reported success`, `READBACK MOTION: MID (1)`, `VERIFIED: intended setting is active.`
- Subsequent **manual STATUS**:
```
STATUS: current radar sensitivity groups:
  Motion  = MID (group 1)
  Presence= MID (group 1)
  Link diagnostics: valid_frames=391 bad=0 notices=377 UART_errors=0 I2C_errors=0
  Original hardware snapshot: MOTION MID, PRESENCE MID.
  NO sensor settings changed by STATUS.
```
- **Result: USER-VERIFIED PASS** for the complete C4002 motion preset control round-trip: original MID (1) -> LOW (0) (write ACK + readback) -> MID (1) (write ACK + readback), then independent STATUS confirms both motion and presence MID and zero communication errors. Last prior GitHub checkpoint saying restoration pending is now SUPERSEDED. Do not assert sensitivity changes were behaviorally calibrated; setting update is proven, not detection-performance curve.
- Candidate source `diagnostics/arduino/slot18_c4002_sensitivity_control/slot18_c4002_sensitivity_control.ino` has been hardware-tested for motion writes, readback and manual STATUS. **Presence writes untested**, along with LOW/HIGH response effectiveness; no automatic sensor calibration attempted.
- User's original baseline remains: Motion MID, Presence MID, detection 0–1100cm, disappearance hold 15 seconds, resolution 80cm. Text config backup `docs/C4002_VERIFIED_CONFIGURATION_2026-11-10.md`.
- Recommended single next test: `PLAN PRESENCE LOW` then review proposed MID -> LOW, `CONFIRM` within 60s, check readback and report; then restore via `PLAN PRESENCE MID` + `CONFIRM` and `STATUS` (separate stage). No code or wiring change; working original continuous monitor and audit intact.

---

# 2026-11-10 — PHASE B FIRST SENSOR WRITE VERIFIED: MOTION MID -> LOW (restoration pending)

- User uploaded Arduino Serial Monitor log `Pasted text(20261111-024217).txt`, 200 lines, showing after successful sensitivity control startup and manual STATUS:
```
CURRENT MOTION: MID (1)
PROPOSED MOTION: MID -> LOW
Type CONFIRM within 60s to change this ONE setting.
Or CANCEL to leave everything unchanged.
APPLYING MOTION MID -> LOW (single radar WRITE)...
WRITE ACK: radar reported success.
READBACK MOTION: LOW (0)
VERIFIED: intended setting is active.
Original hardware baseline is MID for each group.
To restore, PLAN this group MID, then CONFIRM separately.
```
- **User-verified hardware functionality** of read-before, `PLAN`, operator `CONFIRM`, DFRobot 0x87 write response and separate checksum-valid readback on the replacement SEN0691 via CJMCU-752 UART A bridge. Motion LOW group 0 **is the last verified actual value** at end of uploaded log; **DO NOT CLAIM IT HAS BEEN RESTORED YET**.
- Presence was MID at last read-only STATUS, and no presence-setting write is logged. Original baseline includes **MOTION MID 1 / PRESENCE MID 1**, 0–1100cm range, 15-second disappearance delay, 80cm resolution, firmware `CDJQ_A300_1.0.0_251028@JQ_1.0.0_251028`; original values saved in `docs/C4002_VERIFIED_CONFIGURATION_2026-11-10.md`.
- **NEXT: restore immediately** using Serial Monitor newline with `PLAN MOTION MID`, verify it reports `LOW -> MID`, then send `CONFIRM` within 60 seconds. Send `STATUS` and share output, check Motion MID and Presence MID before proceeding. If write or readback warns/fails, don't assume restoration; inspect log and retry only deliberately.
- All three diagnostic sketches still preserved in repo. No source change needed for successful control command. Hardware range and actual detection sensitivity performance not yet calibrated/validated; the write acceptance only validates configuration control. Keep deferred power topic deferred.

---

# 2026-11-10 — Phase B sensitivity control boots, queries both groups: USER-VERIFIED READ-ONLY STARTUP PASS

- User uploaded Serial Monitor paste `Pasted text(20261111-023743).txt`, 183 lines. First t=24..119s prior continuous program, then an ESP-ROM reboot and previous standalone settings-audit 6/6 PASS, then **a second reboot** followed by latest `SLOT-18 C4002 SENSITIVITY CONTROL — CONFIRM REQUIRED` candidate. Interpret only the lines after that second boot as the new control test, not the earlier session lines.
- Actual new program startup:
```
SLOT-18 C4002 SENSITIVITY CONTROL — CONFIRM REQUIRED
No automatic C4002 writes. STATUS is read-only.
Bridge OK; UART A 115200, 8N1.

STATUS: current radar sensitivity groups:
  Motion  = MID (group 1)
  Presence= MID (group 1)
  Link diagnostics: valid_frames=3 bad=0 notices=1 UART_errors=0 I2C_errors=0
  Original hardware snapshot: MOTION MID, PRESENCE MID.
  NO sensor settings changed by STATUS.

COMMANDS (Serial Monitor 115200, Newline):
STATUS; PLAN MOTION LOW|MID|HIGH;
PLAN PRESENCE LOW|MID|HIGH; CONFIRM; CANCEL; HELP
PLAN does NOT change radar; CONFIRM performs ONE write after re-reading.
Restore original group with PLAN MOTION MID or PLAN PRESENCE MID.
```
- **Verification status:** Phase B sensitivity-control candidate `diagnostics/arduino/slot18_c4002_sensitivity_control/slot18_c4002_sensitivity_control.ino` **user-compiled/uploaded, startup and READ-ONLY motion+presence STATUS verified** via CJMCU-752; expected UART read frames working and zero link errors. **No user-input interactive `STATUS` command nor `PLAN` / `CONFIRM` or actual setting writes/readback tested yet**. No sensor settings changed. Baseline values MID / MID are still verified. Existing continuous monitor and read-only settings audit untouched.
- **Guided next test:** send **one serial command `STATUS`** in Arduino Serial Monitor with newline, 115200, to verify interactive command processing; this does not change sensor settings. Once it responds, progress to a single explicitly confirmed motion sensitivity LOW test and independent restoration to MID, with immediate checksum-validated readback; do not adjust presence simultaneously or auto-calibrate. Any unexpected response: stop before writing. Keep 60-second PLAN expiry in mind: do not ask user to PLAN and wait for reply in a future turn; instruct plan-and-confirm as one test only when ready.
- Correct hardware USB-C: Waveshare port labeled UART (CH343) rather than native USB port, which previously led to COM mismatch/blank monitor.

---

# 2026-11-10 — USER VERIFIED: C4002 settings audit PASS, baseline saved; phase B candidate created

**Actual user-uploaded Serial Monitor log:** `Pasted text(20261111-022934).txt` (157 lines). The first ~103 lines are output from previous CONTINUOUS sketch (t=24..119 s); a visible new ESP-ROM boot separator follows, then the NEW read-only settings audit banner. User previously had plugged the native ESP32-S3 USB port, yielding COM5 and blank UART Serial Monitor; switched to correct Waveshare **UART USB-C CH343** and output returned. No firmware or board wiring fault implied.

**Phase A — READ-ONLY settings audit USER-VERIFIED PASS on physical bench**:
- Firmware version (0x82, 40 bytes) `CDJQ_A300_1.0.0_251028@JQ_1.0.0_251028`
- Motion sensitivity (0x87 gate0): MID (1), raw `00 01`
- Presence sensitivity (0x87 gate1): MID (1), raw `01 01`
- Range (0x86): min 0cm max 1100cm (0 to 11.0 meters), raw `00 00 4C 04`
- Disappearance delay (0x84): 15s, raw `0F 00`
- Resolution (0x66): 0 = 80cm, raw `00`.
- Summary exactly `AUDIT COMPLETE: good_settings=6 failed_settings=0 timeouts=0 valid_frames=7 checksum_or_length_errors=0 notifications=1 rx_bytes=167 UART_error_flags=0 I2C_errors=0`.
- No C4002 settings changed. Baseline **saved** at `docs/C4002_VERIFIED_CONFIGURATION_2026-11-10.md`, commit `775612e0044e4f34c0422f1de31ffe9907305c8b`. This is a text snapshot of read configuration only, not a full sensor NVM or firmware image.

**Phase B — new separate operator-confirmed SENSITIVITY control CANDIDATE**, not tested/compiled: `diagnostics/arduino/slot18_c4002_sensitivity_control/slot18_c4002_sensitivity_control.ino`, commit `2544503484f8c8f7e8d150561d3e004b5d9e4617`. It has no automatic C4002 writes: startup queries current MOTION/PRESENCE, commands `STATUS`, `PLAN MOTION LOW|MID|HIGH`, `PLAN PRESENCE LOW|MID|HIGH`, `CONFIRM`, `CANCEL`, `HELP`; PLAN reads current and asks for CONFIRM (expires 60s), CONFIRM rereads/guards initial value, writes ONE selected preset (DFRobot write frame type 0x00 / command 0x87), then separately reads it back (read frame type 0x01 / response 0x03). A write may persist after reboot; no silent factory-reset/auto-calibration/threshold writes; manual MID restoration per channel via PLAN/CONFIRM. Candidate is structurally checked only, **NOT user-run/compiled/verified**, so protect original continuous and read-only audit sketches.

**Recommended single next guided test:** flash new candidate while wiring/USB CH343 unchanged, Serial Monitor 115200, verify initial read-only STATUS shows both MID; send `STATUS` in Serial Monitor using Newline; DO NOT command `PLAN`/`CONFIRM` until basic reads behave as expected and user is ready to test changing one setting. If no serial output check Waveshare UART USB-C CH343 port rather than USB; press RESET after opening Serial Monitor. Report compilation/serial output verbatim. As always user controls power before rewiring. No further sensor config change without deliberate physical user action.

**No changes to working known-good** 17-min `slot18_c4002_continuous.ino` (zero errors) or user-proven `slot18_c4002_settings_audit.ino`. User was off-axis in other room during 17min run; metal racks, microwave/TV/DVR, aluminum-faced paneling and cats contribute reflections/detections; do not call reported presence verified detection of the user.

---

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
