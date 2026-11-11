# 2026-11-10/11 — FIRST USER-VERIFIED LIVE 7-INCH LCD RADAR DASHBOARD (MAJOR MILESTONE)

User installed Arduino ESP32 board package **3.1.3** to resolve `i2c: CONFLICT! driver_ng ... old driver`, set **ESP32S3 Dev Module**, **16MB flash**, **OPI PSRAM (8MB)**, verified and uploaded the v2 sketch `diagnostics/arduino/slot18_c4002_lcd_dashboard/slot18_c4002_lcd_dashboard.ino` (GitHub commit `6249e0869c800a0b05203d267ea0bf9a7ad9f3fb`). **The Waveshare ESP32-S3 Touch LCD 7 SCREEN NOW LIGHTS AND DISPLAYS LIVE RADAR DATA**. User supplied two clear photos of screen. This is first confirmed hardware success, not just compilation/PC serial plot.

Photo A: `SLOT-18 / RADAR LIVE`, `LINK: LIVE`, `PRESENCE DISTANCE 1.43 m`, `MOTION DISTANCE 0.86 m`, `RADAR STATE PRESENCE`, `Valid: 67 Bad: 0 I2C: 0`.
Photo B: `PRESENCE DISTANCE 1.41 m`, `MOTION DISTANCE 1.09 m`, `RADAR STATE PRESENCE`, `LINK: LIVE`, `Valid: 52 Bad: 0 I2C: 0`.
Numbers changed across photos, proving **real C4002 radar-to-CJMCU-752-to-ESP32-to-7-inch LCD live updates**. No error counters in snapshots. Earlier crash caused by Arduino Core 3.3.x with mixed I2C drivers; workaround is using core 3.1.3 **plus** legacy-bus v2 LCD sketch. Exactly which change is essential has not been isolated. Do not casually upgrade board core when testing LCD. Touch intentionally disabled in the board config. Two horizontal progress bars appear as glowing circles near 0m in photos rather than visibly filled bars; investigate styling/range only after stability test. Radar readings are **distance-only**; this is not directional/XY blip display.

**Immediate next single action:** user asks whether to supply Serial Monitor data. Assistant answered YES. Ask for **first 20-30 lines after power-on/reset and several `LIVE:` lines** at 115200 (or paste full log if easier); check for `SLOT-18 / WAVESHARE LIVE RADAR LCD v2 (I2C compatibility)`, `LCD DASHBOARD READY`, `RADAR BRIDGE READY`, sensor streaming, errors and uptime. No wiring or code changes until log reviewed. Prefer concise steps. Then test distance bars and design Aliens-inspired UI while preserving this proven LCD baseline sketch and user-verified standalone radar sketches.

---
# 2026-11-10/11 — LCD v2 STILL CRASHES, now confirmed in user's 56,780-line Serial log; try compatible Arduino ESP32 3.1.3 core next

User uploaded `Pasted text(20261111-061829).txt`, 56,780 lines, ~1.99 MB. Program crashes/reboots **2,183 times**, with same `E (152) i2c: CONFLICT! driver_ng is not allowed to be used with this old driver`. Most captures still include old build SHA `0cd81a299` (2096 instances), **then 88 repeated crashes of NEW build ELF SHA `54208cfba`**, with new abort PC `0x4205569b`; proving that new firmware was indeed uploaded but the crash persists. **No `SLOT-18` startup marker anywhere**, implying abort prior to setup(). Display remained blank. The v2 patch (removed Arduino Wire from sketch, reused legacy IDF I2C bus for CJMCU) was **NOT sufficient**, probably new/legacy I2C code still linked by core or installed library. Do not mark LCD fixed and do not just prescribe a hard reset. `esp_core_dump_flash` CRC warnings are secondary to the crash.

**Externally verified driver issue:** upstream https://github.com/esp-arduino-libs/ESP32_Display_Panel/issues/206 documents the conflicting new driver in Arduino-ESP32 3.2+ with old IO_Expander/Display_Panel. Another detailed https://github.com/esp-arduino-libs/ESP32_Display_Panel/issues/243 describes Arduino 3.3.x NG behavior. The installed ESP32_Display_Panel current Arduino guidance requires esp32 >=3.1.0; **Arduino-ESP32 core version 3.1.3 uses ESP-IDF >=5.3,<5.4 (confirmed https://components.espressif.com/components/espressif/arduino-esp32/versions/3.1.3/dependencies )** and pre-dates switch of Wire driver to new IDF i2c-ng at 5.4. The Waveshare official wiki also supports 3.0+ core for this board (https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-7).

**NEXT single bounded user action:** In Arduino IDE, **Tools > Board > Boards Manager**, search `esp32`, change **esp32 by Espressif Systems** version to **3.1.3**, click Install; keep current LVGL9.5, ESP32_Display_Panel and v2 sketch. This is a TEMPORARY COMPATIBILITY WORKAROUND for the LCD build, not a global project-wide requirement; note board menu selections may reset. After installation, **Verify only**, report success/first error; DO NOT yet upload. If install gives dependency/version errors, capture FIRST error and avoid guessing. If 3.1.3 compiles, upload next and inspect display/Serial for startup. If it still crashes, isolate library integration by compiling Waveshare manufacturer LCD-only example and check installed library versions, or migrate full panel library stack to I2C-ng; do not patch Arduino core files ad hoc.

Preserve proven radar-only `Wire` sketches unmodified. Existing LCD dashboard v2 sketch remains on GitHub at `diagnostics/arduino/slot18_c4002_lcd_dashboard/slot18_c4002_lcd_dashboard.ino` SHA commit `6249e0869c800a0b05203d267ea0bf9a7ad9f3fb`. User prefers short visible atomic milestones. Don't revisit power/wiring.

---

# 2026-11-10/11 — Actual Waveshare 7-inch LCD boot crash diagnosed; candidate v2 uploaded to GitHub (NOT YET TESTED)

User successfully compiled/uploads the original `diagnostics/arduino/slot18_c4002_lcd_dashboard/slot18_c4002_lcd_dashboard.ino` after installing ESP32_Display_Panel, LVGL9.5 and configuring `lv_conf.h`. Waveshare LCD stays blank; Serial log is an abort/reboot loop, ~6553 lines including >250 repeated instances of `E (152) i2c: CONFLICT! driver_ng is not allowed to be used with this old driver`. **No `SLOT-18` marker or app debug lines**; failure occurs prior to setup() due conflicting old and new IDF I2C drivers. Secondary core dump CRC failure on each crash; not evidence of hardware damage. Upstream reproducer: https://github.com/esp-arduino-libs/ESP32_Display_Panel/issues/206.

**ROOT CAUSE in our dashboard:** `Wire.h` links new I2C driver, but existing ESP32_Display_Panel CH422G expander uses legacy IDF I2C. Turning off the expander is NOT an option: it manages display power/backlight. Other proven radar-only sketches using `Wire` are unaffected because they don't link this display library.

**FIX COMMITTED to GitHub dashboard sketch at SHA `6249e0869c800a0b05203d267ea0bf9a7ad9f3fb`:** replaced Arduino `Wire` include and usage with `driver/i2c.h` legacy IDF bus writes and reads (`i2c_master_write_to_device`, `i2c_master_write_read_device`), sharing I2C0 initialized by `board->begin()`, no duplicate host initialization. Leave three LVGL/Waveshare support files and all other code unchanged. README updated SHA `1ab3e980258180a9691c8a7f0e48c7983e06bcea`. Code source structurally inspected; **NOT Arduino compiled, flashed, or hardware verified after patch yet.** It may expose a new compile or runtime issue. Do not claim fixed until user's new evidence.

**NEXT SINGLE USER ACTION:** download latest sketch source file only (GitHub `Raw` then Save As), replace local `C:\\Users\\zuspa\\Desktop\\Slot-18-main\\diagnostics\\arduino\\slot18_c4002_lcd_dashboard\\slot18_c4002_lcd_dashboard.ino` leaving sibling .cpp/.h intact; reopen file in Arduino IDE, **Verify/compile ONLY**, share success or FIRST error. If pass, next step upload and inspect 7in display+Serial `SLOT-18 / WAVESHARE LIVE RADAR LCD v2 (I2C compatibility)`. Do not ask for another hard reset now; the old firmware is predictably crash looping, board not damaged. User prefers small atomic steps due to ADHD; be succinct. User's proven radar settings unchanged.

---
# 2026-11-10 — User's handheld UI direction: Aliens-style scanner + IMU

User says they want a visible **Aliens (1986)-inspired half-circle radar screen with sweeping lines, actual detection marks, plus a settings menu for ranges, calibration, and all existing sensor options**. Handheld movement/orientation should be sensed with an **IMU**, ideally 9DoF but BNO085 I2C has known ESP32-S3 compatibility caveat. Existing C4002 doesn't output XY; **LD2450 can return up to 3 moving targets' X/Y and speed**, which could provide real blips after a separate live sensor test. Never display fictional precise target positions as real. User has ADHD and prefers small visually demonstrable milestones. Full spec: `docs/SLOT18_HANDHELD_RADAR_UI_VISION.md`. **Immediate next test is still the already-prepared LCD dashboard on actual Waveshare hardware**; it is not user-verified. Preserve all working sketches and defer power discussion.

---

# 2026-11-10 — Live Arduino Serial Plotter GUI USER-VERIFIED with screenshot

- User uploaded screenshot `Screenshot 2026-11-10 191918.png` of **Arduino IDE Serial Plotter on COM3** running the Slot-18 C4002 live plotter. User screenshot distinctly shows 4 visible plotted traces and checked legends: `Presence_m` (blue), `Motion_m` (orange), `Radar_state` (green), `Data_OK` (yellow); 115200 baud, RUN.
- **USER-VERIFIED GUI PASS**: live chart displays moving distances; presence roughly 0.7–3.5m in this visible window, intermittent motion values 0–3.5m; radar state transitions 5↔10; link heartbeat `Data_OK` remains 11 during observed screenshot. This is **on PC Arduino Plotter, not yet Waveshare 7-inch LCD**. `Data_OK=11` is deliberate graph-scale marker for recent valid sensor packets, not 11 errors; `Radar_state` 5 presence or 10 motion (non-measured label, not range).
- The prior user text dump independently confirmed live changing numeric notification output; screenshot now additionally proves actual plotted UI is visible and functioning. This does **not** prove specific human ID, through-wall targeting, true XY data, or that all displayed distance values match a physical tape measure.
- **NEXT single visual milestone:** compile/upload separate `diagnostics/arduino/slot18_c4002_lcd_dashboard/slot18_c4002_lcd_dashboard.ino` together with 3 local Waveshare LVGL9 support files; look for large PRESENCE/MOTION distance + bars, status on actual built-in Waveshare 800×480 LCD. LVGL9 + ESP32_Display_Panel dependencies required; first LCD candidate **untested** until user supplies photo or compile error. Do not confuse PC screenshot with Waveshare dashboard success. Existing tested sketches untouched, no wiring changes.

---

# 2026-11-10 — LIVE PLOTTER USER-VERIFIED PASS; FIRST WAVESHARE LCD DASHBOARD CANDIDATE ADDED

## User-provided hardware output (proven)
The user pasted ~170 lines of `slot18_c4002_live_plotter.ino` output with `Data_OK:11` throughout, `Radar_state` switching between 0 (none), 5 (presence), and 10 (motion), and live distances changing. Examples:
```
Presence_m:1.75  Motion_m:0.86  Radar_state:10 Data_OK:11
Presence_m:0.68  Motion_m:1.22  Radar_state:10 Data_OK:11
Presence_m:2.97  Motion_m:0.00  Radar_state:5  Data_OK:11
Presence_m:3.45  Motion_m:0.72  Radar_state:10 Data_OK:11
Presence_m:1.37  Motion_m:0.00  Radar_state:5  Data_OK:11
```
- **USER-VERIFIED PASS for live sensor plotter output and command-free decoder**, changing motion and presence distance, recurring fresh radar notification arrivals. 0.68m repeats in reports; might be gating/reflectors, not a target certainty. `Data_OK:11` is a *non-zero numeric graph marker for recent valid data*, not eleven errors.
- Cannot assert graph software itself was displayed from text alone; source prints plotter-compatible data, likely graphs. No reference to known person or true detection performance from this log.
- User has ADHD and explicitly prefers **visible results**, brief one-step testing, not repeated long settings-only work.

## NEW live LCD dashboard candidate (NOT COMPILED OR USER-TESTED)
- Created completely separate `diagnostics/arduino/slot18_c4002_lcd_dashboard/slot18_c4002_lcd_dashboard.ino` commit `d7c9090cd778359ea819186009dfe6f4cc95b927`, and `README.md` commit `dff9ffe3a2ba1768801867bf599a604133d24cd4`.
- Three local support files (manufacturer original Waveshare LVGL9 sample, retaining Apache license headers):
  - `esp_lv_adapter_arduino.h` commit `c020af63eb5318b21b4e4641be2fa04994eed5d2`
  - `esp_lv_adapter_arduino.cpp` commit `5fe0f6338f4846ad426364ee0fc1d173b49e4347`
  - `esp_panel_board_custom_conf.h` commit `7dec71d16e0f4b3beda0bac3ccf2c7fd93420b82` **touch disabled** via `ESP_OPEN_TOUCH 0` only for first display + shared I2C stress test.
- Based on official manufacturer `waveshareteam/ESP32-S3-Touch-LCD-7/examples/Arduino/examples/10_lvgl_v9_demo` (LVGL9.5 + ESP32_Display_Panel). Board is 800x480 RGB565 and uses I2C pins SDA8/SCL9 shared with CJMCU-752 radar bridge 0x4D. New sketch uses original user-tested notification parser and SC16IS752 UART A. On-screen: big panels for `PRESENCE DISTANCE` and `MOTION DISTANCE`, distance bars, reported radar state, fresh-link indicator and link errors. Does not send any C4002 config commands. Source structurally inspected but **NOT Arduino-compiled, not physically tested**. Potential build/link issues require user compile output and hardware testing. Touch temporarily disabled to avoid bus contention; add interactive controls only later.
- **Next ONE user test**: open [LCD dashboard sketch](https://github.com/auxz2jz/Slot-18/blob/main/diagnostics/arduino/slot18_c4002_lcd_dashboard/slot18_c4002_lcd_dashboard.ino) in Arduino IDE with its three supporting files in same sketch folder; requires ESP32_Display_Panel + LVGL 9.5 and Waveshare ESP32-S3-Touch-LCD-7 Arduino core 3.x. Compile/upload using existing wiring + CH343 UART USB-C. Look for on-board 7-inch screen showing two distance bars and radar LIVE/WAITING. Serial expected `LCD DASHBOARD READY; starting C4002 link...` and `RADAR BRIDGE READY, UART A 115200.`; debug every 5s. Request photo or FIRST compile error. No code/source changes until error observed. Existing five proven config/monitor sketches plus newly user-verified plotter preserved.
- Four config settings left at verified originals: motion MID(1), presence MID(1), range 0..1100cm, delay 15sec. Resolution mode was last read at 80cm and never modified. No power tangent.

---

# 2026-11-10 — Next user-facing milestone: VISUAL LIVE PLOTTER (NEW CANDIDATE, NOT YET HARDWARE TESTED)

- User said to continue and specifically wants **visible/tangible results rather than only hidden configuration milestones**. Deliver short, one-test-at-a-time instructions, ideally an immediately changing visual display, not long feature lists.
- Created NEW, separate read-only Arduino sketch `diagnostics/arduino/slot18_c4002_live_plotter/slot18_c4002_live_plotter.ino` at commit `b1ebc50d9b10ea81f1443cfb5c8ed073533dbaa4`.
- Based on previously user-verified continuous C4002 sensor notification decoder, with exact physical Waveshare ESP32-S3 / CJMCU SC16IS752 I2C 0x4D, SDA GPIO8 SCL GPIO9, UART A 115200. Outputs only plotter-compatible labeled numeric fields every 500ms; no C4002 configuration frame and no modifications to other working sketches. Fields: `Presence_m` (stationary/presence range 0–11m), `Motion_m`, `Radar_state` (0=none/unknown,5=presence report,10=motion report), `Data_OK` (11 if valid notification <=5s ago; else 0). Null/zero is NOT proof that there is physically no target: may be no link, insufficient energy or return. Printed numbers on same scale and axes; watch distance traces change while moving a hand or walking toward/from sensor. Environmental clutter, metal, animals can confound human identification and distance.
- **Not yet Arduino compiled, flashed, or physically tested.** Basic structural checks passed (balanced braces, recovered prior parser/bridge helpers, no radar sensor write calls). The first user test: download sketch; Arduino IDE compile/upload; close Serial Monitor and open **Tools > Serial Plotter** (115200 baud) while using Waveshare **UART CH343 USB-C port**; observe labeled colored graph scrolling live. User can send screenshot or paste plotter values. If no curves, check connection and note whether `Data_OK` line is 11 vs 0. This is on their *PC Serial Plotter*, not the Waveshare built-in touchscreen yet.
- **Future after plotter verified**: move these same live data fields onto the Waveshare's actual 7-inch touchscreen using proper board LCD and LVGL setup; avoid introducing untested display dependencies prematurely.
- Latest sensor baseline confirmed restored: Motion MID1; Presence MID1; Range 0..1100cm; disappearance hold 15s; 80cm resolution (read-only audited); user-verified all four prior config controls complete. Next hardware config resolution test deferred intentionally for visible output.
- User prefers concise single-stage tasks and no power tangent.

---

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

# 2026-11-10 — SENSITIVITY CONTROLLER INTERACTIVE STATUS USER-VERIFIED PASS

- Latest user-uploaded full Arduino Serial Monitor log: `Pasted text(20261111-024042).txt` (190 lines). The log includes old continuous and audit output for context; new content appended after sensitivity-control startup banner.
- **Manual `STATUS` was successfully received and processed** after the startup instructions; new output at lines 184–189:
```
STATUS: current radar sensitivity groups:
  Motion  = MID (group 1)
  Presence= MID (group 1)
  Link diagnostics: valid_frames=184 bad=0 notices=180 UART_errors=0 I2C_errors=0
  Original hardware snapshot: MOTION MID, PRESENCE MID.
  NO sensor settings changed by STATUS.
```
- Status **PASS**: PC UART input -> ESP32 command parser, live sensor READ replies, 180 live notification frames counted, zero I2C/UART/invalid frame errors. Confirmed both original groups still MID, no writes. Working source `diagnostics/arduino/slot18_c4002_sensitivity_control/slot18_c4002_sensitivity_control.ino` compiled/uploaded and now tested for startup + manual read-only command. **WRITE/CONFIRM and readback NOT TESTED YET**.
- **Next controlled milestone:** one explicit `PLAN MOTION LOW` then inspect proposed MID -> LOW; type `CONFIRM` within 60s in same on-hardware session, which should write *motion only* and re-read, followed by `STATUS`. Record exact output, including any warning. For returning to original snapshot later, separate `PLAN MOTION MID` + `CONFIRM` and `STATUS` to verify. Presence must remain MID throughout. If control writes fail, stop and troubleshoot without repeated writes. The user likes one test at a time; do not silently change sensor settings or assume a plan alone changes anything.
- No edits to user-verified continuous monitor or read-only audit. No need to revisit USB hardware port (CH343 UART USB-C verified).

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

# 2026-11-10 — C4002 live detection test environment: clutter/metal/feline targets

- User clarified physical building environment contains **wire shelving/rack, microwave, TV, DVR, many other stored objects, multiple cats**, and **aluminum-faced reflective solar/insulation paneling on approximately half of the building roof**. The user was ~20 ft away in another room off to the right of outward-facing upright C4002. This is not a controlled empty target area.
- These factors introduce plausible **static metal reflectors / multipath and occlusion**; cats may create true animal motion/presence returns. Environmental fans or appliance motion remain only hypotheses, not confirmed user observations. Metal reflections do not alone prove occupancy, and no identification of any reflection/cat/person from C4002 telemetry is possible.
- Reinterpret prior continuous 17-minute run: excellent wired UART communications (`valid=1084, bad=0, I2C_errors=0` at t=1021s) but frequent PRESENCE **cannot be classified as false positives or human through-wall detection** due to clutter and cats. Nominal 120-degree sensor cone is not a proof of absolute off-axis isolation where reflections are possible.
- **Recommended next test:** keep current continuous sketch/wiring unchanged; aim board face at a less cluttered area away from large metal/foil surfaces where possible; compare ~1m/2m/3m measured stationary versus moving returns with pets excluded from test zone where practical, record presence of fans/reflections and NO_TARGET after vacating test area. Do not change radar thresholds/calibration/configuration before the cleaner baseline.
- User specifically requested deferring power supply wiring questions.

---

# 2026-11-10 — C4002 sensor orientation confirmed by user (physical bench)

- User confirms the C4002 SEN0691 board was **upright and vertical**, **gold antenna patches and logo facing outward toward the doorway**, and **pins at the bottom**, essentially as in the earlier reference photo. This answers the mounting/orientation question: forward face was horizontal toward doorway, not pointing at ceiling.
- User was about **20 ft away, in another room, farther off to the right than the previous explanatory sketch**, separated by a wall; exact sensor-to-user bearing, angle, beam coverage through wall, and any intervening objects are NOT measured. The sensor's continued PRESENCE report does NOT prove detecting that user. Radar-reported presence distance often ~1.5-3m vs estimated user separation ~6m, so association is unverified.
- Do not revisit board orientation unless new evidence. Next meaningful test: controlled empty-facing-space observation and measured-distance test to assess background reflections, off-axis detection and false presence. No wiring/code changes authorized or necessary. Single-USB-power question is deferred per user preference.

---

# 2026-11-10 — C4002 board mounting/orientation and test geometry clarification

- User says the SEN0691 C4002 was apparently facing **out a doorway**; the user was about 20ft away in a different room, toward the **right** side of the sensor, separated by a wall, not directly along the sensor boresight. User uncertain about exact radar orientation: they say the printed label was facing forward/up and the 5 pins were at the bottom. **No physical photograph/level measurement from this specific mounting yet.**
- DFRobot official SEN0691 wiki lists 120° x 120° nominal coverage and front-face gold antenna patches visible next to DFRobot label. The PCB should be **vertical with the broad printed/antenna face toward the doorway** for side-mounted horizontal sensing; the label-up/pins-bottom orientation is reasonable **only if PCB plane is vertical**. If board lies flat face-up, its boresight is mainly upward, not outward. Source: https://wiki.dfrobot.com/sen0691/ (manufacturer). Estimated 20ft (~6m) offset to side of boresight may be outside nominal main lobe if angular deviation >~60°; the precise physical angle unknown. This is NOT evidence of through-wall detection or person identification.
- C4002 reported presence at ranges often ~1.5-3m, different from user's estimated 6m distance, reinforcing unidentified foreground reflection/background-target possibility. Don't infer false positives or through-wall performance until controlled empty-field tests.
- User requested defer power supply conversation; continue sensor orientation / detection reliability or display when asked.

---

# 2026-11-10 — Critical new context for 17m C4002 presence report

- User clarified that for much of the prior extended 17min continuous run, **they were ~20 feet away washing dishes in a different room** from the stationary C4002. Therefore repeated `PRESENCE` during that window **MUST NOT be described as successful stationary detection of the user** or as verified true-positive occupancy. The radar may have sensed some other movement/presence, an environmental source, reflections, or possibly detected through a barrier; all remain **unproven**. The sensor logs often reported presence distances nearer ~1.5–3m and motion distance values remained stale when motion energy=0; these measurements are NOT reliable identity/location evidence.
- User requested **set aside power distribution issue for now**; do not repeat USB-C wiring tangent unless they return to it.
- C4002 17min UART reliability PASS unaffected, with zero errors and sustained valid frames. **Target discrimination/false-positive rejection remains open**. DFRobot C4002 supports environmental calibration and distance-gate thresholds per vendor library, but do **not** send configuration/calibration commands or assume they solve it before controlled vacant-room test and measurement. Simple next test: radar aiming at empty room while user elsewhere and no one in the area, logging state for a few minutes; note walls/doors/appliances/HVAC/fans and orientation.
- Source: clarification directly from user in chat after successful test; source repo prior log from Pasted text(20261111-015916).txt.

---

# 2026-11-10 — User-verified continuous C4002 reliability extended run PASS (17m08s)

**Evidence:** user-uploaded serial monitor log `Pasted text(20261111-015916).txt`, 1,134 lines; log contains one active `Slot-18 C4002 CONTINUOUS LIVE MONITOR (READ-ONLY)` session, serial timestamps t=2 through t=1028s (**17m08s**). Sketch: `diagnostics/arduino/slot18_c4002_continuous/slot18_c4002_continuous.ino`, commit `af2e3af25f20d0fa45afe71a04f28b15432188c8`.

**Cumulative 30s health reports:** 34 SUMMARY lines from t=31s through t=1021s; at t=301s (>5 minutes), 325 valid frames, 0 bad and zero UART/I2C errors; at t=601s, 641 valid; final t=1021s `rx=34688 valid=1084 bad=0 detection=1084 overrun=0 parity=0 framing=0 breaks=0 I2C_errors=0 last_detection_age=0s`. Subsequent live readings continue through t=1028s. **No `I2C ERROR`, `NO NEW DATA`, `I2C LINK RESTORED` or `DATA RESUMED` in full log**. Live field outputs include NO_TARGET at session start, MOTION and PRESENCE throughout, AWAY and APPROACHING and varying range/velocity. There are only 33 NO_TARGET lines (~t=2–22s) and none detected later in this log; **do not infer** the area was deliberately vacated for an absence test, nor that later PRESENCE was accurate target association. A movement/distance reading is not calibration; distance=0.00m while PRESENCE and motion values held at stale nonzero speed=0 can occur. No object identification/XY capability.

**Verification status:** continuous monitor **USER-RUN VERIFIED for >17 min, no interface errors observed**; the C4002 UART via SC16IS752 and real radar packets are robust under the recorded test conditions. Earlier intentional breadboard power-offs explain earlier fault logs; the new prolonged run has **zero**. Preserve both the last user-verified original bounded 25s and the newer user-verified continuous sketch; do not change working code unless evidence requires it. Actual single USB-C power distribution has not been verified; red Waveshare I2C VCC lead remains disconnected/insulated.

**Next action:** discuss next small controlled milestone based on user preference: (a) perform measured physical distance / vacant-area check using existing continuous sketch without any code changes, OR (b) bring up Waveshare 7-inch display/touch using official example; previously `0x5D` touchscreen address absent in generic scanner (not proof of damage), OR (c) if user asks for one-supply power, identify exact breakout/wiring/power budgets & voltage before attempting. No current action needed for bridge communication stability. Medium reasoning suitable for routine next development task; increase only if display driver/power fault proves difficult.

---

# 2026-11-10 — User confirmed intentional bridge power-off caused continuous-run I2C gaps

- User explicitly stated they **switched off the breadboard power supply a couple of times while repositioning the setup to prevent damage** during the continuous C4002 monitor recording. The monitor's I2C errors at approximately t=15s in one run and t=135s in another are therefore **consistent with intentionally removing CJMCU-752 power while ESP32 USB/monitor remained live**. These log entries do not on their own establish module failure or an intermittent electrical defect.
- Keep prior raw counts, including 3870 I2C failed-read attempts, 1 overrun and 1 framing error in the first session; interpret them in the context of power cycling. The second continuous session provided **134 valid radar frames with zero checksum/UART/I2C errors through 121s**; verified NO_TARGET then MOTION/PRESENCE events, plus continuous 30s summaries.
- **Status:** C4002 wired UART A and read-only continuous monitor **USER-VERIFIED functional** for multiple minutes while powered. Long-duration uninterrupted reliability and range accuracy not yet formally validated. Do not revise code or change hardware based on deliberate outages.
- **Recommended next action:** keep all connections stationary and both board and bridge powered for a longer uninterrupted bench run (e.g. 3–5 minutes) with at least several SUMMARY lines, then tape-measured ranges and presence/motion absence; observe any new errors *without power changes*. Fully disconnect both USB and breadboard power before rewiring or repositioning the powered circuit; leave the screen red I2C VCC lead insulated.
- Continue using `diagnostics/arduino/slot18_c4002_continuous/slot18_c4002_continuous.ino`; last user-verified bounded 25s sketch preserved separately. Screen custom UI/touch controller 0x5D and LD2450 remain separate work.

---

# 2026-11-10 — Continuous C4002 physical test: sustained data PASS; intermittent CJMCU I2C outage UNKNOWN CAUSE

**Source:** user-uploaded `Pasted text(20261111-012731).txt` (319 lines), user's own live Serial Monitor evidence; no user-stated circumstances of power removal/unplugging. Separate previous bounded 25s logs appear at file start; interpret **TWO** independent continuous monitor startup banners, not one uninterrupted run.

**Continuous monitor (new sketch, first run):**
- Boot at line ~90, CJMCU ACK and valid sensor frames through ~t=15s. Then repeated `I2C ERROR: unable to read CJMCU-752 line status`.
- Later `I2C LINK RESTORED` occurred but valid notifications did NOT resume in the captured section; `NO NEW DATA for 5 seconds` printed.
- 209s summary: `rx=513 valid=16 bad=0 detection=16 overrun=1 parity=0 framing=1 breaks=0 I2C_errors=3870 last_detection_age=193s`.
- Cause unknown. A deliberate breadboard power off, shaky external 5V/GND, intermittent SDA/SCL, or software/electrical instability are possibilities; **do not conclude bridge hardware defective** without asking user what was happening when failures occurred. Count repeated error print lines is not same as internal I2C attempt count.

**Continuous monitor (second fresh startup at file line ~158):**
- Starts with `NO_TARGET` t=2–11s, then valid `MOTION` and `PRESENCE` with plausible changing speed/distance, including AWAY and APPROACHING. `NO_TARGET` valid packet implies radar itself reported no target; distinguish from absent data.
- Summaries:
  - 31s: `rx=1216 valid=38 bad=0 detection=38 overrun=0 parity=0 framing=0 breaks=0 I2C_errors=0`.
  - 61s: `rx=2272 valid=71 bad=0 detection=71 overrun=0 parity=0 framing=0 breaks=0 I2C_errors=0`.
  - 91s: `rx=3232 valid=101 bad=0 detection=101 overrun=0 parity=0 framing=0 breaks=0 I2C_errors=0`.
  - 121s: `rx=4288 valid=134 bad=0 detection=134 overrun=0 parity=0 framing=0 breaks=0 I2C_errors=0`.
- Valid live readings through ~t=135s; file ends with a single `I2C ERROR` afterward, without later context.
- **CANDIDATE now user-run physical test PASS for indefinite loop past 120s and actual observed NO_TARGET + PRESENCE/MOTION packets**; baseline 25s remains preserved. **Not yet verified indefinitely stable or range-accurate**; unexplained I2C outages remain open. Ranges and motion values are radar-reported not calibrated, and stale motion distances occur when energy=0 / status PRESENCE.
- **Next atomic test**: Ask user whether they intentionally disconnected or switched off CJMCU + radar breadboard supply at ~15s during first continuous run and ~135s in second, or physically disturbed wiring. If intentional, record expected disconnect behavior, avoid code changes. If unintentional, user should power OFF and inspect/verify supply rails, GND, I2C SDA/SCL seating/polarity, then perform 2–5 min stationary no-touch power-on test, recording failures; diagnose first failure without reflashing. Screen's red I2C VCC remains disconnected and insulated. No touch UI testing yet.
- **No source modification this turn**, only diagnostic evidence; current continuous sketch path `diagnostics/arduino/slot18_c4002_continuous/slot18_c4002_continuous.ino`, source commit `af2e3af25f20d0fa45afe71a04f28b15432188c8`.

---

# 2026-11-10 — Continuous C4002 monitor candidate ready (S18-DIAG-20261110-CONTINUOUS-01)

- **User request:** replace 25-second bounded live diagnostic with **continuous Serial Monitor observation**, using same verified 1.8432MHz/115200-8N1 CJMCU-752 bridge and C4002 replacement radar wiring. User previously confirmed radar stationary while they moved hand up/down and toward/away. No physical movement/range calibration yet.
- **Created:** `diagnostics/arduino/slot18_c4002_continuous/slot18_c4002_continuous.ino` commit `af2e3af25f20d0fa45afe71a04f28b15432188c8`. A separate ~220-line standalone Arduino sketch cloned from the **user-verified working 25-second** `slot18_c4002_live_readonly.ino` (which was NOT edited). Continues capturing each checksum-valid C4002 detection notification and prints presence, motion, range, speed, energy and direction in Serial Monitor; explicit NO_TARGET hides potentially stale distances; prints 30s cumulative checksum/UART/I2C summary, warnings for >5s missing sensor packets and throttled bridge I2C errors, and resumed-link messages. It makes no C4002 configuration writes; only initializes the bridge UART A, same as baseline. No LCD UI.
- **Status: CANDIDATE / USER HARDWARE TEST PENDING.** Source retrieved from GitHub and statically reviewed for retained UART config/packet parser and indefinite `loop()`; Arduino compile and hardware upload have NOT been performed by assistant. The previous user-verified 25-second sketch remains baseline and can be reuploaded if needed.
- **Exact next user action:** Arduino IDE open raw `https://raw.githubusercontent.com/auxz2jz/Slot-18/main/diagnostics/arduino/slot18_c4002_continuous/slot18_c4002_continuous.ino`, select board Waveshare ESP32-S3-Touch-LCD-7; COM3; Flash 16MB; 16MB 3MB APP / 9.9MB FATFS; PSRAM Enabled; Serial Monitor 115200. Replace ALL editor text, Verify then Upload. Watch stream for 1-2 minutes with hand moving, then leave area for 20-30s and observe any NO_TARGET. 30s SUMMARY must continue; `NO NEW DATA` means transport gap, *not* radar's no-target report. Report output and real setup, do not repeat unrelated I2C scanner or UART version probe. Monitor closing does not stop firmware; power-down or reflash to stop.
- **Next task after user verification:** compare measured distance 1/2/3m, then separately resolve missing touchscreen 0x5D and begin screen UI. Normal/Medium coding reasoning sufficient; escalate only for a recurrent unexpected bridge/flash fault.

---

# 2026-11-10 — Physical test conditions clarified by user

During the user-run 25-second C4002 live capture (32 valid frames, 0 errors), **the radar remained stationary**. User was **seated nearby and moving a hand up/down and toward/away from the sensor**. Do not characterize this capture as user walking or a measured-distance calibration. PRESENCE/MOTION transitions are consistent with motion in the scene but cannot identify which reflected object produced each reported range or prove hand/person identity. Next guided test: stationary seated interval, controlled hand toward/away motion, then vacate target area and check eventual NO_TARGET; later compare readings at tape-measured 1/2/3m positions. No new hardware/code change or new sensor validation from this clarification.

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

# 2026-11-10 — Clock crystal confirmed; UART probe candidate ready (TEST PENDING)

- User supplied close-up photo and explicitly read **1.8432 MHz** marking on the silver crystal of the physical CJMCU-752 SC16IS752 board. This resolves S18-HW-20261027-01 oscillator ambiguity. Per NXP clock/divisor equation with prescaler=1, **divisor=1 → 115200 baud, 8N1**. Clock frequency is supported by marking but real oscillator signal has not been measured independently.
- The original failed UART capture on the OLD C4002 did test this same 1.8432 MHz hypothetical setting; the failure was *not proven caused by the clock*. Old sensor has since been replaced. New replacement's C4002 UART has not yet been physically tested.
- New one-shot bounded read-only Arduino test candidate: `diagnostics/arduino/slot18_c4002_uart_probe/slot18_c4002_uart_probe.ino` (commit `3c66547eb8353ec2b20054c1e13f5de685e6e9fa`). It sets bridge UART A divisor 1 and 8N1, logs I2C ACK, captures passive RX with line-status flags, sends one correctly framed manufacturer **read-only software-version request** (cmd 0x82), then captures bytes/error flags and validates FA F5 AA A5 + length + 16bit additive checksum. No radar config writes, resets, endless retries, or display initialization. Source **NOT compiled, flashed, or tested** by assistant/user.
- **Next single test**: With wiring power-off and rail polarity verified, ensure C4002 TX→CJMCU RXA and RX→TXA, common GND, separate screen USB, red screen I2C VCC insulated. In Arduino IDE keep 16MB flash, matching 16MB partition scheme, PSRAM enabled, board profile, COM3, serial 115200. Upload new probe once; copy complete one-shot Serial Monitor result. If no valid frame, inspect line error flags, RX activity, physical TX/RX idle via DMM/logic analyzer before other attempts. Do not mark radar UART or detection verified until valid checksum frame.
- Prior replacement-bench result still valid: bridge responds `0x4D` and 26 I2C addresses reported; touchscreen `0x5D` absent in scan; keep touch diagnostic independent from C4002 UART.

---

# 2026-11-10 — Replacement C4002 bench: I2C ACK verified, external UART pending

**New user-reported test:** After correcting Arduino flash settings to 16MB flash, 16MB (3MB APP / 9.9MB FATFS) partition scheme, and enabling PSRAM, the read-only `slot18_i2c_discovery` scanner now runs (previous 8MB boot-partition mismatch resolved in this upload). User reported 25 responding I2C addresses before powering the external CJMCU-752/C4002 setup, then 26 with the setup powered; scanner printed `ACK 0x4D (CJMCU-752 expected)` and `CJMCU-752 0x4D: PASS (I2C ACK only)`. **External CJMCU I2C address connection PASSED for this bench run.** Unexpectedly, `Touch 0x5D: NOT FOUND` was printed; the complete address list has not yet been provided, so do NOT infer touch failure or assume exactly which 25 were previously present. Note previous factory touch demo was user-verified before flashing serial diagnostics.

**NOT yet established:** replacement C4002 UART communication or any verified radar reading. C4002's RUN/OUT LEDs are not proof of UART validity. Next atomic step per October 27 handoff: photograph/read physical CJMCU-752 oscillator/crystal marking (with power OFF) before choosing the correct UART baud divisor, or examine a complete single I2C scan if addressing issue needs immediate follow-up. Do not repeat two guessed clock settings. Preserve boot and I2C results in state.

---

# Slot-18 — 2026-11-10 user serial log: ESP32 boot loop, I2C test BLOCKED

- User uploaded Arduino Serial Monitor capture (20,783 lines), not committed because bulk noisy logs stay attached to chat.
- Boot continuously reports: `partition 4 invalid - offset 0x610000 size 0x9e0000 exceeds flash chip size 0x800000`, then `Failed to verify partition table` and `load partition table error!`. Approx. 1,598 repeats of the reset banner and 1,597–1,598 repeats of each error. **No `Slot-18 I2C Bridge Discovery` output; diagnostic NEVER REACHED setup.**
- Prior direct esptool interrogation reported physical flash 16MB, N16R8; current bootloader is enforcing 8MB (`0x800000`) while partition table spans nearly 16MB. Suspect incorrect Arduino Flash Size setting or bootloader image configuration (8MB vs actual 16MB), NOT proven component failure.
- **Next atomic action:** In Arduino IDE Tools, confirm board profile, `Flash Size = 16MB (128Mb)` and a matching 16MB partition scheme, plus OPI PSRAM as previously verified. Re-upload existing read-only I2C scanner (do NOT rewrite code, do NOT erase all flash); capture fresh serial output. If menu does not expose these values, inspect selected board profile rather than guessing. Do not cycle same failed build.
- Screen blank is expected for scanner, but serial should show its own banner when boot works. Do not test C4002 until scanner boots, then verify `0x4D` bridge and `0x5D` touch.
- Previously saved original full 16MB factory image remains private on Windows and restore untested. Preserve it.
- No replacement-radar results established by this failed boot.

---

# NEXT CHAT HANDOFF — Slot-18 — 2026-11-10 (NEW SENSOR / TEST PENDING)

**Read this note first, then the detailed 2026-10-27 checkpoint below.** User has replaced the DFRobot C4002 SEN0691 radar with another C4002 of the same model. User reports RUN and OUT indicator activity on the replacement. This is NOT a verified UART handshake, detection, or power-voltage measurement on the replacement. Waveshare screen and CJMCU-752 bridge are unchanged. Screen's loose **red I2C VCC lead must remain DISCONNECTED and insulated** while screen is powered via its USB-C; share ground/SDA/SCL only. Recheck actual breadboard rail polarity with meter before powering the replacement (old reversed-rail incident). No new user-verified C4002 serial success.

**Current atomic diagnostic:** Confirm PC/Arduino IDE port and verify I2C bus after reassembly using new read-only diagnostic candidate at `diagnostics/arduino/slot18_i2c_discovery/slot18_i2c_discovery.ino`. Expected CJMCU I2C ACK `0x4D` and touch controller `0x5D`; scan alone does not test external radar UART, make the display show UI, or validate radar reporting. Program not yet compiled or run by user; test results pending. If the I2C path passes, return to task **S18-HW-20261027-01**: inspect actual SC16IS752 breakout oscillator marking (rather than reusing guessed oscillator settings), then bounded UART status/framing test at confirmed divisor; mark PASS only on valid framed/checksummed C4002 data. No sensor configuration resets or unrelated code changes.

**Repository change:** diagnostic sketch committed as `106cadfda990bc63e0b3beb523e19eef43140807` (source candidate, not a physical test). Previous 16MB factory flash backup still on user's Windows machine; restore untested. Previous factory display/touch PASS and SC16IS752 internal loopbacks PASS remain historical evidence; no new production firmware compile or live radar UI. Continue one test at a time; obey anti-loop rule.

---

# NEXT CHAT HANDOFF — Slot-18 — 2026-10-27 (LATEST / READ FIRST)

**TASK:** User requested "make a handoff and save all your progress to the GitHub for starting a new conversation." THIS IS A DOCUMENTATION-ONLY CHECKPOINT. Do not resume flashing, rewiring, building, researching products or editing working code until instructed in the next conversation.

**Repository:** https://github.com/auxz2jz/Slot-18 — default branch main.
**Detailed authoritative physical bench log:** [docs/2026-10-27_PHYSICAL_BENCH_CHECKPOINT.md](docs/2026-10-27_PHYSICAL_BENCH_CHECKPOINT.md).
**Development rules:** [AGENTS.md](AGENTS.md) and https://github.com/auxz2jz/master-instruction-library. Read rule index, core recovery, diagnostics, guided testing, Codex efficiency, and cross-platform standard before code changes. Project memory and roadmap follow.

## CURRENT EXACT STATE (supersedes older "hardware NONE" below)

User HAS assembled and physically tested Waveshare ESP32-S3-Touch-LCD-7 Rev1.2 N16R8 on USB-C, and one DFRobot C4002 radar via purple SC16IS752 CJMCU-752 I²C-to-UART board on externally powered breadboard. Factory Waveshare screen demo powered up and touch/typing **user-verified**. Factory demo's Wi-Fi/SD self-tests reported FAILED (not diagnosed). User owns an untested LD2450 radar, second CJMCU-752, spare small breadboard and supply, plus oversized ABS project enclosure.

The factory demo was backed up to the user's Windows PC *before* uploading serial-only Arduino diagnostics. Complete 16MB `waveshare_factory_16MB.bin`, **16,777,216 bytes**, SHA256 `2791680bf3923804b04b55b237aea83e38af3981a9c224ab7ddc0b4cf6684a21`, location `%USERPROFILE%\Documents\Slot18_Backup\`. Backed-up flash **not restore-tested**. NEVER commit binary/private credentials. Display now likely **blank** while diagnostic sketch runs; this is expected and not evidence that display failed.

Current hardware: screen physical UART selector UART1; programming USB-C UART1 through Windows **CH343 COM3**; ESP32-S3, 16MB flash and 8MB PSRAM verified by esptool v5.6.0 (Arduino output used separate esptool v5.3.1); Windows Python 3.14.9. Native Arduino board profile `Waveshare ESP32-S3-Touch-LCD-7` with 16MB flash, PSRAM enabled. Rev1.2 touchscreen I²C voltage is factory solder-strap on **3V3** (NOT plug-in jumper); screen I²C SDA GPIO8, SCL GPIO9. Screen's I²C red VCC disconnected/insulated, common GND wire connected to breadboard, touchscreen powered separately via USB-C. Breadboard + rail voltage 4.9V measured at both sensor and bridge VCC; initial red/blue rail mismatch was corrected. User soldered C4002 and bridge headers. Physical pin wiring as described by user is in detailed document.

### PASS / NOT YET PASS

| Diagnostic | Evidence and meaning |
|---|---|
| Waveshare factory screen/touch | **USER-VERIFIED PASS**: graphics, buttons, typing. Factory Wi-Fi + SD self-check FAILED/unknown cause |
| C4002 powered | **PASS**: ~4.9V at VIN/GND; LED blinking, no obvious static VIN–GND short; actual radar detection unverified |
| CJMCU powered | **PASS**: ~4.9V at VCC/GND; no obvious static short |
| Arduino upload / serial COM3 | **PASS**, backup performed and checksum recorded before uploads |
| I²C discovery | **PASS**: UART bridge ACK at `0x4D`; onboard `0x20–0x27`, `0x30–0x3F`, `0x5D` also present |
| Both bridge UART scratchpad registers | **PASS**: `UART A: REGISTER TEST PASS`; `UART B: REGISTER TEST PASS` |
| Both UART internal loopbacks | **PASS**: `UART A: LOOPBACK PASS`; `UART B: LOOPBACK PASS`; proves internal chip path only |
| **Actual C4002 UART communication** | **NOT VERIFIED**: read-only C4002 version request did not return validated frame |
| ESP-IDF production v0.2.0 | **UNVERIFIED SOURCE CANDIDATE**. CI previously failed on missing `esp_random.h` include, still not fixed; no tested built production image |
| Browser prototype v0.1.0 | Automated tests passed before current chat; not user visually verified |

### Current blocking evidence / next task S18-HW-20261027-01

Most recent serial sketch sent **C4002 version query** with SC16IS752 UART A and tested two hypothesized oscillator/divisor settings. Serial output:
```text
Slot-18 C4002 UART Diagnostic
Testing crystal: 1.8432 MHz
Received 66 UART bytes
47 5E F0 DF 9E 06 87 00 ... (garbled; no FA F5 AA A5 validated header)
No verified version response
Testing crystal: 14.7456 MHz
Received 8 UART bytes
00 00 00 00 00 00 00 00
No verified version response
Diagnostic complete.
```
**User's LATEST measurement immediately before this handoff:** two multimeters, one red probe C4002 TX black C4002 GND, the other red probe SC16IS752 RXA black bridge GND, both readings fluctuating ~0.5–1.0V. This is **not a UART byte capture or proof** of correct serial signal integrity, logic high, 115200 baud or detection. No reset/command change done to radar.

**FIRST NEXT ACTION ONLY after requested resume:** verify marking/clock frequency of actual CJMCU-752 crystal, then use bounded line status and RX capture test at matching divisor / protocol; potentially isolate TTL TX line or use USB-to-TTL analyzer. Check whether sensor UART TX idle has sensible 3.3V logic, and if physical UART wiring or stale bytes are cause. Do **NOT** rerun the same ineffective two-clock sketch without new evidence, and do not assume reverse-polarity damaged the C4002. Mark PASS only on correct checksum/framed sensor response. Then move to live C4002 motion/presence/range readings (never invent XY coordinates), then separately LD2450.

**Safety:** Switch power OFF before wiring; verify breadboard positive & negative with meter (user previously discovered the HW-131 mounted reversed relative to colored rails). Do NOT tie touchscreen USB VCC to externally driven positive 5V rail. Common grounds yes. Keep additional sensors disconnected while troubleshooting.

### Reconciled longer-term plans — not implemented

User has in hand a project box that proved much too large; would prefer a smaller DIY ABS box. Wants radars mounted behind uncoated ABS without antenna obstruction; nylon adhesive PCB clips/standoffs may be used, C4002 mounting holes ~M2; LD2450 edge clips preferable. Future visible + IR/NoIR + thermal image overlay/switching, with desired person thermal detection 50–100 ft, thermal optics/higher-res camera still research only. Prior discussion *changed away from* bulky custom hot-swappable 1S4P battery cartridges to ordinary protected 18650 cells (8–16 handheld, 4–8 remote), eventual 12V barrel shared vehicle/wall input, regulated protected buses, charge while operating. Exact circuit, battery series/parallel, BMS, fuses, cameras, thermal, power-path **UNSELECTED AND UNTESTED**; historical power architecture in the prior handoff below is superseded as user intent and must not be implemented blindly. For initial bench, keep main screen on USB-C and radars on verified 5V supply.

### Handoff instructions for NEXT chat

Read THIS latest block and [detailed bench checkpoint](docs/2026-10-27_PHYSICAL_BENCH_CHECKPOINT.md), then project-specific and global rules. Treat **only the factory display/touch baseline** as user-verified visually, and UART register/loopback tests as user-supplied tool diagnostics. No Slot-18 working custom live radar UI exists. Diagnostic sketches live on user's Windows Arduino IDE, not committed to firmware/esp-idf. Do not claim a restore from full-flash BIN was exercised. Preserve test evidence; make one atomic step at a time, obey anti-loop. Suggested next model/reasoning: medium for single hardware probe; high only if persistent multi-source driver/oscillator/protocol ambiguity.

---

## HISTORICAL HANDOFF — 2026-10-09 (ARCHIVED; superseded status, retain old design notes)

# NEXT CHAT HANDOFF — Slot-18 — 2026-10-09

**Mode:** CHECKPOINT ONLY. The user explicitly said **"Stop you're in a loop"** and then instructed: **"we reached limit for this chat please save all work and the GitHub save your current progress and prepare for another chat to take over."** No further coding, repeated research, battery purchases or unasked changes are authorized in this handoff.

**Canonical project:** https://github.com/auxz2jz/Slot-18 (branch `main`)
**Master rules:** https://github.com/auxz2jz/master-instruction-library — read `INSTRUCTION_INDEX.md`, `CORE_DEVELOPMENT_RECOVERY_RULES.md`, `DIAGNOSTICS_STANDARD.md`, `GUIDED_TESTING_STANDARD.md`, `CODEX_USAGE_EFFICIENCY_STANDARD.md`, and `CROSS_PLATFORM_COLLABORATION_STANDARD.md`. Then `AGENTS.md`, `PROJECT_MEMORY.md`, `ROADMAP.md`, and the relevant design and test files.

## 1. Stop/recovery rule and source of truth
- **STOP LOOP. CHECKPOINT ONLY.** No need to re-explain the same battery-power basics or browse the same boards repeatedly. This document supersedes any stale assumptions scattered through earlier design files as a **status and intent checkpoint**, but does **not** silently declare hardware designs finalized.
- User requested saving and handoff, **not** a build or more research right now.
- **Last user-VERIFIED hardware/software baseline: NONE.** No physical ESP32, radar, battery pack, charging PCB, power combiner or hot-swap test has been confirmed. Do not infer hardware was purchased.
- **Current software candidates:** `prototype/` browser simulated radar v0.1.0; earlier CI core test PASS, but no reported user visual verification. `firmware/esp-idf/` v0.2.0 ESP32-S3 touchscreen simulated-only candidate with source originally introduced at commit `7ef78bcfb7ffac51acfb310cc08c679223b583d1`. Both remain unverified by user; no actual radar input from hardware.
- **ESP-IDF CI is failing, not pending:** latest observed run at https://github.com/auxz2jz/Slot-18/actions/runs/37726404321, concluded FAILURE (source `31ec1237e6c525c369f41ae9a8b0a4460f51db7e`). Inspecting build log identified `firmware/esp-idf/main/main.c:182:16: error: implicit declaration of function 'esp_random'`; build log hint: manually add `#include "esp_random.h"` since it is no longer included from `esp_system.h`. **NOT FIXED in this checkpoint.** Earlier initial failure was incompatible ESP-IDF 5.4.x / esp_lvgl_adapter ^0.5.2; CI workflow was updated to ESP-IDF 5.5.2, then uncovered this C compilation error. Do not claim there is a successfully built v0.2.0 binary or APK. The browser `Core tests` action at https://github.com/auxz2jz/Slot-18/actions/runs/37724357604 succeeded (automated test only).
- Current firmware hardware first test is still manufacturer's Waveshare display/touch demo, then the Slot-18 simulated screen, then LD2450 sensor. Initial bench uses safe regulated 5V USB wall power, NOT battery or raw mains.
- Firmware and prototype source were **not edited** during this final checkpoint; only documentation is saved.

## 2. Product scope beyond batteries
- Main controller/display: Waveshare ESP32-S3-Touch-LCD-7, N16R8, 7in 800x480. Radar sensors originally planned: Hi-Link LD2450 for moving XY tracks, DFRobot C4001 SEN0609 (~25m), C4002 SEN0691 (presence). Later additional radar/thermal/LiDAR/IMU/GNSS, directional tower and up to 5+ remote stations.
- The sensor capability model distinguishes **real vs simulated**, and real XY vs range/presence-only with unknown position. Do not plot invented target positions.
- Remote station links: local ESP-NOW/Wi-Fi, **true multihop mesh** with relaying still future/unverified, last-seen/packet IDs. Android native companion, local web dashboard over ESP32 Wi-Fi AP or local LAN, BLE setup/light status, and optional encrypted internet gateway are planned ONLY. No APK or connected dashboard built yet.
- Separate Android / web / firmware workspaces in repository; global cross-platform standards apply. All stations send battery status: pack presence, each SoC% and optional SoH%, voltage/current/temperature where measured, charging/fault/source, estimated remaining runtime, low battery; export to handheld then Android/web.

## 3. BATTERY SYSTEM — user requirements and actual decisions
- User clarified configuration **1S4P**: **four matched 18650 Li-ion cells in PARALLEL within each removable cartridge** (~3.6/3.7V nominal; for nominal 3Ah/cell, ~12Ah and ~43Wh nominal per cartridge). User once called it "4S1P" by mistake; assistant clarified, and all planning uses 1S4P unless user explicitly changes it.
- **Hot-swappable unit is the ENTIRE four-cell cartridge.** Never swap individual cells while cartridge installed or powered. User wants cartridges openable to service cells after removal; **do not represent arbitrary individual replacement as safe** in an electrically hard-paralleled pack. Matched cells, compatible state of charge/capacity, real protection and professional pack handling necessary; prefer replacing matched sets if degraded.
- Each **remote station has TWO four-cell cartridges** (= 8 cells). Main handheld **TWO initially, expandable to FOUR such cartridges** (= 8–16 cells), subject to runtime/weight. Any ONE installed healthy cartridge must run the device's worst measured peak load; any 1/2/3/4 can be installed in handheld. Power interruptions when removing one must NOT reset device, subject to actual tested circuit.
- Each cartridge must provide:
  (a) 1S pack BMS/protection, temperature management/fuse, correct Lithium CC/CV charging;
  (b) **charge OUTSIDE the device** via cartridge USB-C **or** a spring-contact/drop-in cradle;
  (c) **charge INSIDE the device** via an independent charging input contact supplied from the remote/handheld USB-C wall input or optional solar manager;
  (d) independently protected power DISCHARGE output through contacts to the host;
  (e) safe keyed/recessed/durable dock or pogo pins/approved blind-mate terminal; status/pack identity monitoring where supported.
- The **two charging modes mean outside vs inside the device**, not two charger ICs active at the same time on the same battery. Charging through USB-C, cradle, or installed contacts may use ONE correctly chosen charger per cartridge with input mux/protection. The port/input pads and connector compatibility remain unverified.
- The **device must charge cartridges while simultaneously operating**. Simplest modular system: external USB / regulated solar first powers the load directly, while independent current-limited charge feeds go to each docked pack. Cartridge protected outputs separately feed backup power to a common device supply. Do NOT use the OR-ed battery discharge bus as a battery charging input and do not create pack-to-pack backfeed or external input-to-output loops. Device power is stable regulated 5V (additional 3.3V rail if needed for remote boards).
- Device external power: wall/USB-C can power device when batteries are absent if adapter rated enough; optional PV has its own solar regulator/MPPT or dynamic power control and a selection/backfeed strategy. Available charger power = surplus after device load, with per-pack throttling/sequencing if necessary. Do not assume panels provide nominal watts continuously.
- **Do not claim single PCB per pack/device already selected, purchased or tested**; two broad architectural alternatives are still open:
  **Option A**: protected 1S pack -> per-cartridge charger + 5V boost/power-path -> one separately reverse-current blocked 5V OR input per cartridge -> common regulated 5V backup bus -> external-priority power MUX. Upside standalone cartridge is USB power bank; risk board sleep and charge-mode output interruption.
  **Option B** (emerged as attractive): each protected 1S4P pack has a SEPARATE charger branch and raw protected battery-voltage discharge connection; device ORs independent ~3–4.2V pack outputs through correctly rated reverse-blocking/hot-plug protection -> ONE central adequately rated boost to 5V -> external-priority system MUX. Need higher current connector/boost/OR at low voltage, pack health monitoring, and rigorous load-step/hot-swap tests. This architecture avoids independent power-bank sleep and multiple individual boost outputs. **Not yet adopted or validated.**
- A safe **ideal diode OR-ing stage** is valid in principle to combine cartridge outputs with common negative ground and blocked positive paths; it does NOT automatically share current equally, add all output board current limits, charge packs, wake sleeping converters, or guarantee no dropout during hot insertion/removal. A pack must handle full system load. Every cartridge channel still needs controlled hot-plug/inrush/current/short protection and meaningful failover verification. Input/output positive/negative, physical shell and temperatures matter.
- Possible IC candidates: Analog Devices LTC4412 (ideal diode controller with MOSFET), TI TPS2121 (2-input power MUX), TI TPS25947 (eFuse with reverse-current blocking), higher-current 1S charger candidates TI BQ25620/BQ25895, optional BQ25798 (1–4 cells **in series** per single pack, not a multi-cartridge charger). These are NOT complete verified Amazon plug-together systems.
- **IP5328P 18W USB-C PD power-bank module** was discussed as cartridge candidate, with full 5V output, charging. Key drawback: **standard implementations turn off boost output while charging**, may sleep on low loads, so not automatically acceptable for 24/7 continuous pass-through and hot-swap. One earlier Amazon example was https://www.amazon.com/dp/B09L3WNQ22 ; compatibility NOT verified.
- **Waveshare Solar Power Manager (D)** is a candidate with manufacturer-advertised simultaneous charge and discharge, 5V/3A output, USB-C 5V input and 6–24V solar input, battery input designed for 1S lithium. Source: https://www.waveshare.com/solar-power-manager-d.htm ; https://www.waveshare.com/wiki/Solar_Power_Manager_%28D%29 ; possible Amazon link https://www.amazon.com/dp/B0CT2ZBML9 but stock and exact SKU not assured. Critical **unresolved**: separate dockable charging-input connection vs USB-C port, actual battery charge current, 1S4P ~12Ah compatibility, continuous 5V failover with both plugged/unplugged, solar charge prioritization, true USB-C PD *input* (DO NOT assume; USB-C input documented 5V), and any manufacturer distinction between simultaneous standard operation and high-speed modes. **The user had asked a full independent investigation of this Waveshare board, then said "Stop you're in a loop." Do not claim that requested detailed manufacturer review was completed.**
- User wants **Amazon availability for all purchased hardware**, ideally exact verified Amazon product links rather than search-only placeholders. Previous assistant has repeatedly suggested generic modules but none have been confirmed jointly compatible or purchased. **Do not resume an endless shopping/research loop.** Next chat should focus on one falsifiable design question, stop after evidence, and offer actual tested circuit topology only when supported.

## 4. CAD and mechanics
- The case must contain screen board, 1/2 or 2/4 cartridges, controller, sensor brackets, RF-clear windows, safety/heat clearance, USB-C charging and optional solar connectors. Prefer parametric 3D-print design and replaceable brackets. Mechanical reference docs: `docs/ENCLOSURE_MECHANICAL_REFERENCE.md` with Waveshare mount patterns / sensor board sizes. No case 3D model created in repository or fit-verified.
- Each printed cartridge has four protected 18650 cells, insertion rails/latch, recessed 5V or raw-pack output contacts (based on final architecture), SEPARATE charging contacts, polarity keying, and serviceable lid for removed/offline pack only. Pogo/PCB contacts must have current and mating-cycle ratings, safe/short-proof arrangement, and spacing. Safety requires protected PCB, appropriate holder and wiring; printed plastic alone is NOT a fire-safe Li-ion pack.

## 5. Actual file map / where to continue
- Core status: `PROJECT_MEMORY.md`, `ROADMAP.md`, `AGENTS.md`, `README.md`
- Main power architecture: `docs/USB_SOLAR_MULTIPACK_POWER_ARCHITECTURE.md`
- Device power and per-pack charge isolation / alternate 3.7V OR vs 5V OR: `docs/DEVICE_SIDE_POWER_BACKPLANE.md`
- Inventory/buy-and-build gates: `docs/POWER_SYSTEM_BUILD_CHECKLIST.md`
- Energy budget: `docs/18650_MULTICELL_POWER_SIZING.md`
- Earlier pouch research only (SUPERSEDED FORM-FACTOR, NOT deleted): `docs/HOT_SWAPPABLE_LIPO_POWER_SYSTEM.md`
- Case/sensor dimensions: `docs/ENCLOSURE_MECHANICAL_REFERENCE.md`, `docs/HARDWARE_AND_SENSORS.md`
- Networking/Android/web: `docs/REMOTE_ACCESS_AND_CLIENTS.md`, `shared/FEATURE_CATALOG.md`, `shared/DATA_FORMATS_AND_INTERFACES.md`, `android/README.md`, `web/README.md`
- Firmware and diagnostics: `firmware/esp-idf/`, `docs/FIRST_HARDWARE_TEST.md`, `docs/DIAGNOSTICS_AND_TESTING.md`, `.github/workflows/esp-idf-build.yml`.
- All user requests about code or hardware remain plans unless actually tested. Do not mark anything COMPLETE or VERIFIED without user evidence.

## 6. Exact next action for the NEXT chat (do not perform during checkpoint)
**First confirm receipt of this saved handoff, report baseline NONE and the unresolved state briefly, then ask the user which track to resume only when requested.** If user asks to resume the **battery research** specifically, take ONE narrowly scoped first step: obtain manufacturer primary documentation for **Waveshare Solar Power Manager (D)**, verify battery and port specs, charging current, charging + regulated 5V output under load, charge input separate from boosted output, and changeover behavior. Write an evidence matrix from primary source, explicitly flag unknowns, and decide whether a separate charger + raw 1S power OR design is safer. No bulk shopping or code edits until that evidence. If user asks instead to resume firmware, targeted next fix is `#include "esp_random.h"` for failing IDF v0.2.0 compile, followed by a new candidate CI run with logs and NO claim of hardware validation.

**No automatic continuation or background work. Resume only on new user direction.**
