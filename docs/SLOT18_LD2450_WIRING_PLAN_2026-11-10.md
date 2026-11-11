# Slot-18 / HLK-LD2450 staged wiring plan (2026-11-10/11)

Status: **PLANNED ONLY — NOT WIRED, NOT TESTED.** Preserve the verified v2 Waveshare 7-inch LCD and C4002 UART-A sketch.

## Confirmed electrical and protocol facts
- HLK-LD2450 pinout (read printed board labels, do not infer wire colors): **5V**, **GND**, **TX**, **RX**. Regulated 5V power source capable of **>200 mA**. Serial I/O is **3.3V** (NOT 5V), default **256000 baud, 8N1**; emits up to 3 moving target X/Y positions and speed at ~10Hz.
- Factory 256000 can be changed to 115200 via manufacturer config command 0x00A1 (baud index 0x0005) after entering config (0x00FF); effective after reboot. Never change until baseline data verified.
- User owns CP2102 USB-to-TTL. CP2102 supports 256000 baud natively, suitable for *standalone sensor handshake and decode first*.
- Waveshare ESP32-S3-Touch-LCD-7 (800x480), external UART2 HY2.0 4P shares **UART0 GPIO43/44** with CH343 UART1 USB-C via physical slide switch. UART2 header **3V3 is NOT LD2450's required 5V supply**. Avoid sacrificing Serial Monitor by using this header in first iteration.

## Proposed integration
1. **Unpowered standalone CP2102 test**: LD2450 TX→CP2102 RX, LD2450 RX→CP2102 TX, GND→GND. Supply LD2450 5V from a **verified regulated 5V source rated >200mA** and share ground. Never use a 3V3 pin for radar power. CP2102 adapters differ: verify that one's 5V rail/current and 3.3V IO levels before enabling power. Photograph/read labels if not sure. No Waveshare wiring changed.
2. With a separate sensor test at default 256000 and verified valid framed data, optionally change LD2450 to 115200, restart, and verify.
3. Check actual SC16IS752/CJMCU-752 UART B output high voltage first: earlier bridge supply showed 5V **VCCA**, may have 5V UART TX. Do not connect TXB output straight to 3.3V LD2450 RX if it is 5V; use divider or level translator or receive-only first. Verify LD2450 3.3V TX meets UART-B input VIH.
4. Final design: C4002 on bridge UART-A (already user-proven), LD2450 on UART-B (115200), shared I2C (address 0x4D bridge) into Waveshare LVGL app, separate verified 5V radar rail, common GND. Develop distinct candidate sketch preserving known-good LCD v2 and read-only sensor settings. Test simultaneous 24GHz radar interference.
5. Future UI directional plot from LD2450 **real X/Y** targets; C4002 reports only presence/motion distances, no azimuth.

## Source references
- LD2450 datasheet: https://manuals.plus/m/9725e6c2408e7de5b444a21528c902743d1d4777d01871ad8bb1ade086cd7300
- LD2450 config and X/Y frames: https://manuals.plus/m/9725e6c2408e7de5b444a21528c902743d1d4777d01871ad8bb1ade086cd7300
- CP2102/9 supported baud list incl 256000: https://www.silabs.com/documents/public/data-sheets/CP2102-9.pdf
- Waveshare UART switch/headers FAQ: https://docs.waveshare.com/ESP32-S3-Touch-LCD-7/FAQ
