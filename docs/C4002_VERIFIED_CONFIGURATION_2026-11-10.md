# C4002/SEN0691 — Original settings verified on user hardware (2026-11-10)

## Verification status

**USER-VERIFIED READ-ONLY CONFIGURATION**. The user uploaded a Serial Monitor capture named `Pasted text(20261111-022934).txt` after resolving a USB-C port mix-up (native ESP32-S3 USB vs CH343 USB-to-UART). The independent sketch `diagnostics/arduino/slot18_c4002_settings_audit/slot18_c4002_settings_audit.ino` returned **6 of 6 matching, checksum-valid successful responses**. No C4002 configuration changed. This file is a **human-readable snapshot of queried fields**, not a complete firmware/NVM image or automatically restorable backup.

- Board: Waveshare ESP32-S3 Touch LCD 7, original CH343 UART USB-C port for Serial Monitor
- Bridge: CJMCU-752 SC16IS752 I2C address 0x4D, UART A 115200 8N1, marked 1.8432MHz crystal and divisor 1
- Sensor: replacement DFRobot C4002 SEN0691, PCB antennas/label facing outward and pins at bottom
- Firmware text from READ cmd 0x82, 40-byte payload: `CDJQ_A300_1.0.0_251028@JQ_1.0.0_251028`
- Motion threshold group (READ cmd 0x87, gate=0): **1 = MID**, raw `00 01`
- Presence threshold group (READ cmd 0x87, gate=1): **1 = MID**, raw `01 01`
- Detection range (READ cmd 0x86): **0 cm to 1100 cm**, raw `00 00 4C 04`
- Target disappearance delay (READ cmd 0x84): **15 s**, raw `0F 00`
- Range gate resolution (READ cmd 0x66): **code 0, 80 cm**, raw `00`

Last Serial Monitor evidence:
```
AUDIT COMPLETE: good_settings=6 failed_settings=0 timeouts=0 valid_frames=7 checksum_or_length_errors=0 notifications=1 rx_bytes=167 UART_error_flags=0 I2C_errors=0
READ ONLY: no C4002 settings were changed.
```

The text preceding reset in the same paste (t=24–119s PRESENCE/MOTION continuous) is from the **previous continuous-monitor firmware session**, not the configuration audit. A visible ESP-ROM boot header then separates the new sketch output.

## Restorable baseline targets (only after future protocol write/readback are tested)

- `Motion sensitivity: MID (group 1)`
- `Presence sensitivity: MID (group 1)`
- `Min range: 0 cm`
- `Max range: 1100 cm`
- `Disappear delay: 15 seconds`
- `Resolution: 80 cm (code 0)`

Do **not** assume the snapshot includes every sensor setting or that a generic factory reset restores these values. For future changes: record exact before/after reads; allow one setting write at a time with explicit user authorization and checksum-valid readback; provide a separate verified restore path to these values.

## Environment and interpretation

Cluttered building with wire rack, microwave, TV, DVR, cats and aluminum-faced reflective roof insulation. User often off-axis in another room roughly 20 feet away. Detection range 11m covers distances beyond the user's estimated separation, but that **does not establish** which objects/animals the radar detects, direct through-wall reception, false positive or correct target classification.

## Next steps

1. Preserve the read-only audit sketch and continuous live-monitor known-good rollback unchanged.
2. Implement **separate** explicit-command-only sensitivity adjustment sketch (motion vs presence) with read-before and read-after; require an operator command before changes.
3. Test an isolated single-setting change at a time and return to recorded MID when done; do not run environment calibration yet.
4. Later add touchscreen controls after actual LCD/touch bring-up.
