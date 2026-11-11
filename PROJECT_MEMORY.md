# 2026-11-10/11 — LCD live radar Serial log independently confirms FIRST stable integrated demo

User supplied the COMPLETE startup log and ~44 periodic `LIVE:` records for the newly running v2 LCD firmware after selecting **Arduino ESP32 core 3.1.3**, **16MB flash**, **OPI PSRAM**, and uploading. Exact startup:
```
SLOT-18 / WAVESHARE LIVE RADAR LCD v2 (I2C compatibility)
DISPLAY ONLY: no radar configuration writes.
[I][Panel] ... Board initialize success
[I][Expander] ... esp_io_expander_new_i2c_ch422g): version: 0.1.0
[I][Panel] ... Board begin success
LCD DASHBOARD READY; starting C4002 link...
RADAR BRIDGE READY, UART A 115200.
```
Then continuous, increasing, *valid* count from 7 to **257**, **bad=0**, **I2C_errors=0** throughout; presence from 0.68m to 3.16m and motion from 0 to 2.51m in supplied excerpt, state codes 1=Presence, 2=Motion per source code. **No abort, reboot, fatal errors** in complete provided serial snippet. Together with user's two photos confirming `LINK: LIVE` and changing values, first end-to-end on-device C4002 -> CJMCU-752 -> ESP32-S3 -> LVGL on Waveshare 800x480 display is NOW VERIFIED. Onboard LCD graphics rendering, sensor reading, and link stable during sampled period; touchscreen intentionally disabled. Do not change known-good configuration or old standalone test sketches.

**Visual observation for future development:** small glowing circles appear near left of both bars even while ranges show ~1-3m; source has `lv_obj_set_pos(bar,14,195)`, range 0..1100 cm, bar width310, update `centiP` / `centiM`; must separately verify bar behavior, orientation, and dynamic rendering. Distances fluctuate and presence sometimes hits 0.68m frequently; assess environmental calibration and field testing later; don't equate valid frames with measurement accuracy.

**NEXT** once user desires: verify whether colored bars change lengths when changing distances; then preserve current v2 baseline / tag as verified, design a separate copy for more sophisticated Aliens-inspired radar screen (semicircular sweep and distance-based visualization while C4002 has no direction). Could incrementally enable GT911 touchscreen later after considering shared I2C bus. User prefers tiny, high-signal steps, not broad diagnostic checklists. User currently simply pasted Serial log for validation; response should confirm, not ask for more logs.

---

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

# 2026-11-10 — Continuous C4002 candidate (S18-DIAG-20261110-CONTINUOUS-01)

Continuous UART diagnostic source: `diagnostics/arduino/slot18_c4002_continuous/slot18_c4002_continuous.ino` (new GitHub commit `af2e3af25f20d0fa45afe71a04f28b15432188c8`, ~220 lines), **CANDIDATE/UNCOMPILED/NOT YET USER-TESTED**. Preserves the last USER-VERIFIED 25-second original `diagnostics/arduino/slot18_c4002_live_readonly/slot18_c4002_live_readonly.ino` unchanged. Uses same proven CJMCU-752 0x4D, 1.8432MHz crystal, UART A at 115200 8N1; prints continuously, reports NO_TARGET separately from NO DATA (5s), 30s counters and throttled I2C faults; NO radar configuration commands. See NEXT_CHAT_HANDOFF and docs/DIAGNOSTICS_AND_TESTING.md for verification guide. Next: verify and upload, watch still/hand-movement then vacate area, check sustained 30s summaries and no UART errors; then measured distance accuracy. Previous physical test 32/32 valid is baseline only. Recommended next coding model: Medium reasoning; escalate if evidence indicates intermittent I2C/UART loss.

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

# 2026-11-10 replacement C4002 and read-only I2C diagnostic (CURRENT)

- User replaced the previously unresponsive DFRobot C4002 SEN0691 with the same model. Reports RUN and OUT indicator activity only. Replacement's UART, presence and range not tested; power rail voltage on replacement not yet remeasured.
- Same Waveshare 7in board and CJMCU-752 bridge remain. Red I2C VCC wire on screen is intentionally DISCONNECTED and should be insulated; connect only common GND, SDA and SCL. Screen powered by USB-C. Never assume colored breadboard rail polarity is correct.
- New `diagnostics/arduino/slot18_i2c_discovery/slot18_i2c_discovery.ino` source candidate saved on GitHub, commit `106cadfda990bc63e0b3beb523e19eef43140807`. It scans I2C for 0x4D bridge and 0x5D touchscreen, no C4002 UART requests. NOT compiled/flashed/user-verified.
- Next: run read-only scan, capture serial output; if bridge path passes inspect actual module crystal marking then targeted C4002 serial diagnostics. Main blocker remains S18-HW-20261027-01. Prior failed C4002 test applies only to old sensor.
- No new custom display firmware or live radar UI. Preserve original 16MB factory firmware private Windows backup.

---

# Current project memory — physical bench checkpoint (2026-10-27)

**Read `NEXT_CHAT_HANDOFF.md` FIRST.** Detailed hardware results: [docs/2026-10-27_PHYSICAL_BENCH_CHECKPOINT.md](docs/2026-10-27_PHYSICAL_BENCH_CHECKPOINT.md). This section supersedes dated "hardware NONE"/"no user verified baseline" statements below while preserving them as historical records.

- **Task ID:** S18-HW-20261027-01, current blocker: C4002 UART communication through SC16IS752. User requested CHECKPOINT ONLY / new conversation.
- **User-VERIFIED physical baseline:** Waveshare ESP32-S3-Touch-LCD-7 Rev1.2 **factory** touchscreen demo powered and touch/typing worked (factory Wi-Fi/SD self-check FAILED). **NOT** a verified Slot-18 application firmware version.
- **Known recovery artifact:** user's local original 16MB flash backup `waveshare_factory_16MB.bin` at `%USERPROFILE%\Documents\Slot18_Backup\`; byte count 16777216, SHA256 `2791680bf3923804b04b55b237aea83e38af3981a9c224ab7ddc0b4cf6684a21`. Restoration NOT tested; original is NOT in GitHub.
- **Working physical benches:** C4002 and CJMCU-752 on +4.9V breadboard power with common GND and separated touchscreen USB supply; Arduino IDE Waveshare native profile, CH343 UART1 COM3. Screen I2C at 3.3V soldered selector. I2C device at 0x4D; bridge UART A/B scratchpad PASS; bridge A/B internal loopback PASS. Full readings in detailed checkpoint.
- **Latest failure:** C4002 read-only version UART query over CJMCU received 66 unframed/garbled bytes on 1.8432MHz hypothetical clock or 8 zero bytes on 14.7456MHz hypothetical clock; neither passes response header/checksum. Actual board crystal not read from marking. Last user multimeter readings C4002 TX–GND and CJMCU RXA–GND fluctuate 0.5–1.0V on both instruments; UART byte correctness NOT determined, radar presence/distance NOT verified.
- **Equipment physically received:** one Hi-Link LD2450 V1.1 (untested), one DFRobot C4002 SEN0691 (powered only, serial unresolved), two purple CJMCU-752 dual-UART bridges (one in bench test), breadboards 830/400, two HW-131-type power units, jumper wires, multimeter(s), oversized ABS project case. Initial reversed breadboard-colored power rails discovered and corrected; confirm polarity before wiring.
- **Repository production code unchanged in this chat:** browser v0.1.0 candidate automated tests passed previously, firmware ESP-IDF v0.2.0 CI still fails from `esp_random.h` missing declaration; no user-verified custom live radar UI or firmware package. User Arduino sketches exist on Windows locally only.
- **Last explicit direction:** write handoff, update GitHub, **STOP** for new conversation. Next if authorized: verify actual CJMCU oscillator before correct 115200 baud and C4002 frame capture, not repeated guesses. One bounded diagnostic at a time.
- **Prior design update:** original 1S4P hot-swappable battery cartridge architecture in October 9 documents is historical and superseded by user preference for normal 18650 cells (handheld ~8–16, remotes ~4–8), unselected protection/power-path; no verified battery build. User has enclosure too big, future cameras/thermal are still planned.

---

## Earlier historical project memory (through 2026-10-09)

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
