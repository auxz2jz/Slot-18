# Slot-18 physical bench evidence and Arduino diagnostics — 2026-10-27

**Status:** User-observed hardware testing, documentation-only checkpoint. Do not interpret these results as validation of the ESP-IDF v0.2.0 Slot-18 firmware, a live display UI, or an operational radar driver. The current serial-only Arduino diagnostics replaced the factory demo on the board. **No production code or firmware files were changed in GitHub during this chat.**

## Actual hardware the user physically confirmed
- Waveshare ESP32-S3-Touch-LCD-7 Rev1.2 (N16R8; 7-inch 800x480). Factory demo started, display rendered and user successfully tapped buttons and typed. Factory demo Wi-Fi and SD tests reported FAILED, which was not troubleshot; lack of SD card is possible, NOT confirmed as root cause.
- One Hi-Link HLK-LD2450 V1.1 (photos show built-in PCB antennas, 8-pin header, white 4-pin connector and u.FL-style antenna connector), physically in hand **not yet tested**.
- One DFRobot C4002 (SEN0691) Motion & Static Presence 24 GHz radar, with user-soldered 5-pin header. 5-pin silk: VIN / GND / RX / TX / OUT.
- Two purple CJMCU-752 modules bearing SC16IS752 dual-UART chips (at least the active module's headers user-soldered); one currently in use.
- Breadboards 830-point and 400-point, two HW-131-type breadboard power modules with nominal selectable 3.3V/5V, jumper wires, multimeter(s), oversized ABS plastic project enclosure. No secure permanent enclosure mounts yet.
- User intends to use a smaller enclosure next revision; existing box is too large. Adhesive nylon board clips / M2 C4002 supports discussed, no confirmed mounts purchased/installed.
- Future: visible, IR/night vision, thermal cameras and higher-resolution thermal detection (50–100ft target); not purchased or integrated. New power discussion favored regular 18650 cells (8–16 handheld, 4–8 remote), with a common protected USB-C 5V power bus eventually. Old large hot-swap cartridge architecture is not an approved present build.

## Bench wiring CONFIRMED BY USER DESCRIPTION (not physical continuity-audited)
Power module: nominal 5V rails initially mounted in reverse relative to breadboard red/blue color labels. User corrected orientation and measured approximately **4.9V** at C4002 and CJMCU VCC independently. Earlier sensor may briefly have seen reverse polarity; no damage established or excluded. Inspect markings and rail separation, do not trust colors alone.

C4002:
- VIN red -> breadboard +5V; GND black -> common GND
- RX purple -> CJMCU TXA
- TX gray -> CJMCU RXA
- OUT left unconnected

CJMCU:
- VCC purple -> breadboard +5V; GND blue -> common GND
- A0/CS green -> common GND; A1/SI yellow -> common GND (intended address 0x4D)
- I2C/SPI orange -> breadboard +5V (mode select on this regulator-equipped breakout; **do not generalize to bare 3.3V SC16IS752 modules**)
- SCL/SCLK red -> white extension -> touchscreen SCL
- SDA/VSS brown -> yellow extension -> touchscreen SDA
- Channel B not wired to a radar; RESET/IRQ/other GPIO not used.

Waveshare:
- I2C four-pin HY2.0 connector silk: VCC / GND / SDA / SCL. Screen's gray SCL and yellow SDA (per earlier photo), black GND to common breadboard ground; **red VCC cable disconnected/insulated**; screen powered separately via USB-C.
- Rev1.2 zero-ohm solder resistor under I2C 3V3/5V selector mounted on **3V3** position in photo; do not search for a removable jumper.
- For programming: separate USB-C **UART1** port, physical UART switch UART1, CH343 USB serial **COM3**. Distinguish the other port marked USB and the UART switch.

## User-verified tests
1. Screen factory demo: powered, display graphics and capacitive touch/typing worked. Factory Wi-Fi / SD self-check failure unexplained. The screen is now running serial-only diagnostic Arduino sketches, so screen blank is expected; **no live radar UI user-verified**.
2. C4002 power ~4.9V and its LED (user called OUT/RUN) blinking. VIN/GND unpowered multimeter continuity test showed no continuous beep. OUT measured 0.02–0.03V, up to 0.09V with movement—far too inconclusive to call digital detection.
3. CJMCU VCC/GND no continuous continuity beep; powered VCC approximately 4.9V, regulator 3.3V output not independently measured.
4. Windows Python 3.14.9, esptool 5.6.0 via `py -m esptool`. `flash-id` reported ESP32-S3 revision 0.2, 16MB quad flash at 3.3V and 8MB embedded PSRAM. COM3 CH343 recognized.
5. **Factory firmware backup:** user ran `py -m esptool --chip esp32s3 --port COM3 -b 115200 read-flash 0 0x1000000 waveshare_factory_16MB.bin`. Output showed **16,777,216 bytes** read. Local path `%USERPROFILE%\Documents\Slot18_Backup\waveshare_factory_16MB.bin`. SHA-256: `2791680bf3923804b04b55b237aea83e38af3981a9c224ab7ddc0b4cf6684a21`. Keep backup PRIVATE and duplicate offline; **RESTORE HAS NOT BEEN TESTED**, so don't claim recovery guaranteed. Never commit flash image, MAC address or credentials to GitHub.
6. Arduino IDE: Espressif esp32 core 3.3.12 per compile upload log; Arduino board selection `Waveshare ESP32-S3-Touch-LCD-7` (native board profile), 16MB flash, PSRAM enabled, USB CDC on boot disabled, hardware CDC/JTAG mode, COM3 UART1. Sketch uploads passed hash verification.
7. **I2C scan PASS:** `Wire.begin(8,9,100000)`. Onboard CH422G responders 0x20–0x27 and 0x30–0x3F, touchscreen controller 0x5D, **CJMCU 0x4D detected repeatedly**.
8. **SC16IS752 scratchpad test PASS:** UART A and B scratchpad write/read/restore printed `UART A: REGISTER TEST PASS` and `UART B: REGISTER TEST PASS`. Register addressing via `(reg << 3)|(channel << 1)`, I2C address 0x4D.
9. **SC16IS752 internal loopback PASS:** on each channel set UART 8N1; FCR FIFO enabled and reset; MCR bit4 internal loopback, transmit test byte (A 0x55 / B 0xAA), read LSR data-ready/RHR, disable loopback. Printed `UART A: LOOPBACK PASS` and `UART B: LOOPBACK PASS`. This tests inside the UART bridge; **does not prove physical TXA/RXA wiring or C4002 UART link**.
10. **C4002 firmware/version request — NOT VERIFIED (diagnostic failed to receive valid reply):** Arduino diagnostic attempted C4002 read-only 0x82 software version request, serial 115200, UART A via bridge at 0x4D, trying 1.8432 MHz/divisor 1 and 14.7456 MHz/divisor 8. Exact log tail:
```text
Slot-18 C4002 UART Diagnostic

Testing crystal: 1.8432 MHz
Received 66 UART bytes
47 5E F0 DF 9E 06 87 00 47 5E F0 8F 8E 16 D7 00
77 4E F0 CF 9E 06 D7 00 47 5E F0 CF 8E 06 D7 00
47 5E F0 CF 8E 06 D7 00 77 5E F0 D7 9E 16 C7 00
77 5E F0 DF 9E 16 C7 00 77 4E F0 CF 9E 06 D7 00
47 5E
No verified version response

Testing crystal: 14.7456 MHz
Received 8 UART bytes
00 00 00 00 00 00 00 00
No verified version response
Diagnostic complete.
```
11. **Latest physical measurement (after diagnostic):** user used *two multimeters simultaneously* with red probe on **C4002 TX** and red probe on **CJMCU RXA**, black probes on each board's own GND. Both readings fluctuated approximately **0.5–1.0V**. This is consistent with averaged switching signal or other conditions but does **NOT** verify logic levels, correct baud, framing, data integrity or even proper RXA connectivity. Avoid treating multimeter average as UART byte data.

## Current blocker and first next task — S18-HW-20261027-01
**Unresolved link:** SC16IS752 UART A physical receiving from C4002 and decoding valid response. User specifically asked for HANDOFF ONLY now; no more instructions to wire, flash or repeat testing without new permission.

After new chat loads `NEXT_CHAT_HANDOFF.md`, use a **different, evidence-led** step rather than rerun identical failed version scan:
1. Verify board's oscillator/crystal frequency from **sharp close-up of the silver metal crystal on the actual purple CJMCU-752** or measured frequency if equipment allows. The prior program merely guessed two candidate crystals.
2. Inspect C4002 TX idle level with multimeter DC (idle should be logic high on a working TTL UART, but DMM dynamics/loads can obscure), and CJMCU RXA pin under the same conditions with power source verified. Confirm no incorrect rail wiring or shorts, then if needed isolate UART line and compare. Prefer USB-TTL analyzer/logic analyzer/oscilloscope for meaningful UART verification, not voltage averaging.
3. Audit SC16IS752 baud configuration incl. actual crystal and 8N1, RX FIFO status/line status/overrun or break flags, and DFRobot C4002 read-only version framing/checksum against **manufacturer protocol**; use bounded capture, timestamped bytes, report both expected and observed.
4. Check the radar's default UART baud and whether automatic reporting must be enabled for data; do not reset/change radar configuration until positive link diagnosis. The radar may have been briefly reverse-powered due to earlier rail mismatch; note as possibility, not conclusion.
5. Distinct outcome PASS only when valid, framed, checksum-verified C4002 serial messages returned, optionally parsed version or presence/range, then test walking movement; no invented X/Y position for presence-only device.

User's tested Arduino sketches were entered in Arduino IDE during this chat; their exact source is in the chat transcript **but was not automatically uploaded to this GitHub repo**. Production `firmware/esp-idf/` still has documented compile failure at `main.c:182` (`esp_random.h` missing); do not present Arduino sketches as completed production firmware.

## Recovery / external docs
- Read root `NEXT_CHAT_HANDOFF.md` FIRST, `AGENTS.md`, `PROJECT_MEMORY.md`, `ROADMAP.md`; follow `auxz2jz/master-instruction-library` mandatory guidelines.
- GitHub repo `https://github.com/auxz2jz/Slot-18`; local Arduino diagnostic script and original BIN on user's own Windows PC, not available in repository.
- Master anti-loop rule: if repeated failure, STOP; inspect evidence, do not indefinitely cycle same scripts.
- Next task reasoning suggestion: medium for inspecting crystal and one diagnostic; high only if protocol/bridge timing remains persistently ambiguous. Stop after a single bounded task and checkpoint.
