# Slot-18 — One 5V source for 7-inch display and both 24GHz radars (plan only)

**STATUS**: Proposed hardware design, **not wired or tested**. LD2450 still on standalone CP2102 COM4 at 256000 baud. C4002 and Waveshare LCD v2 are user-verified and must be preserved.

## Requirements / verified references
- Waveshare ESP32-S3-Touch-LCD-7 800x480: 5V Type-C power, published consumption ~450mA. Manufacturer https://docs.waveshare.com/ESP32-S3-Touch-LCD-7
- HLK-LD2450: 5V, 120mA average, manufacturer requires supply >200mA. https://www.tinytronics.nl/product_files/006000_HLK-LD2450-Instruction-Manual.pdf
- DFRobot SEN0691 C4002: input 3.6–5.5V, use 5V; actual measured current pending https://wiki.dfrobot.com/sen0691/
- CJMCU-752 SC16IS752 purple I2C/UART bridge: user earlier used 5V VCCA, but check breakout board's regulator, VCCA/VCCB and digital logic before final power and TXB connections. Bridge consumes modest unknown current, not characterized.
- Recommended **external regulated 5V 3A** source initially, or 5V 4A dedicated power supply with margin for future expansion. Do not assume a USB-C device marked 30W can deliver 30W at 5V; check USB-C **5V output current** specifically. Do not use USB-C PD higher-voltage (9/12/20V) triggers without conversion.
- Existing Waveshare single-cell PH2.0 battery header supports only regulated/protected single-cell 3.7V Li-ion/LiPo with <=2000mAh manufacturer recommendation; **do not attach raw multi-cell 18650 pack or 5V to that port**. https://docs.waveshare.com/ESP32-S3-Touch-LCD-7/Instructions-For-Use
- Suggested bank example Anker A1384 20K 30W: verified USB-C **5V 3A** output via Anker own specification https://service.anker.com/product-description/a08J1000000YcHbIAK. Another 20K bank 5V3A similarly works. Anker current price not established from source.
- 5V 4A regulated barrel supply example Mean Well GST25A05-P1J 5V4A center positive, 5.5x2.1mm connector, confirm jack/polarity. Not a charger for 3.7V battery.
- USB-C power-input breakout: should include **both 5.1k Rd CC sink resistors** (e.g. Adafruit #4090), but #4090 explicitly only advertises up to 1.5A and thus should **not** be selected as a guaranteed 3A power distribution inlet without checking rating. https://www.adafruit.com/product/4090
- Protected distribution board: 5V common positive and ground, separate appropriately rated resettable fuse/eFuse per branch (display, C4002, LD2450, bridge), switch on input, decoupling and proper strain relief. Choose actual fuses based on measured load/transients; don't claim USB hub ports all independently fused if not specified.

## Recommended architecture

```
[ONE active input: 5V USB-C bank/adapter OR regulated 5V barrel PSU]
     -> polarity-protected input/CC-compliant connector
     -> suitably rated ON/OFF switch
     -> overcurrent protected 5V distribution (+ / GND)
        -> branch 1: Waveshare via proper USB-C male 5V power cable
        -> branch 2: C4002 VIN and GND
        -> branch 3: LD2450 5V and GND
        -> branch 4: CJMCU-752 appropriate supply pins (check VCCA/VCCB first)
```
All grounds common; UART and I2C are DATA, not a substitute for power distribution. Power each sensor separately from power distribution rail instead of drawing from 3.3V pin of LCD or UART/I2C connector. Keep 3.3V signal-level requirements; do not connect 5V logic to 3.3V RX.

**Critical:** USB-UART debugger + power bank may backfeed board if simultaneously plugged in. Use appropriately isolated USB data cable, validated power path, or disconnect external power while USB programming; no direct tie of multiple USB 5V outputs. **Do NOT plug the USB power bank and wall adapter into the same 5V bus in parallel** unless proper power mux/ideal diode used. A barrel inlet can be alternate input to this same 5V bus through source selector and protection. A panel-mounted enclosure power jack/cable needs strain relief and accessible switch.

### Next step
Before selecting hardware, choose portable bank external vs inside white ABS enclosure, and identify how user will retain USB serial programming with independent power present. Measure actual total peak 5V current when all modules integrated before committing output ratings; user already has multimeter. For first prototype, one regulated 5V supply at 3A and four protected distribution outputs; no permanent enclosure modifications until wiring proven.

## User-selected enclosure architecture (2026-11-11)

**Decision:** The USB power bank will live INSIDE the white ABS enclosure and use TWO separate USB outputs: port 1 feeds Waveshare LCD via its USB-C 5V power input; port 2 feeds a USB-C **5V sink breakout** plus protected distribution to LD2450, C4002 and CJMCU-752 bridge. User initially suggested a USB-C PD trigger module set to 5V. At a 5V-only goal a USB-C sink breakout with correct CC pull-down resistors is simpler than a PD trigger; a PD trigger can also work if fixed at 5V and verified. Important: 5V positive outputs from bank ports MUST NOT be tied together; common ground required between display and sensors for UART/I2C. USB multiport bank needs simultaneous-output and 5V combined current verification, especially if second port has low-current cutoff. When sizing a breakout, check 5V current rating and cable/port ratings, recommended >=2A on sensor branch (3A desired). Choose separate protected distribution branch for each radar/bridge. Don't use 9/12/20V PD modes without regulator. USB-C sink must have appropriate CC resistors (5.1k pulldowns CC1, CC2) for C-to-C operation. Physical design needs removable/accessible bank, strain relief, ventilation, and service access to charging. Confirm whether bank permits charging while powering loads (do not assume pass-through). Do not simultaneously connect PC USB plus battery-supplied VBUS unless backfeed safely managed. No wiring modification yet; pending bank and breakout selection, current measurements.

Sources: Renesas USB Type-C power roles https://www.renesas.com/en/support/engineer-school/usb-power-delivery-03 ; Waveshare model and 5V Type-C https://docs.waveshare.com/ESP32-S3-Touch-LCD-7 .

