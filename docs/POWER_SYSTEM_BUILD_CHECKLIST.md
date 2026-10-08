# Power-System Build Checklist — Current Design vs Missing Hardware

Project: Slot-18 Handheld Multi-Sensor Detection System
Checkpoint: 2026-10-08
STATUS: **SPECIFICATIONS AND CANDIDATE BOARDS ONLY. No user-verified physical battery/charging/power circuit, no confirmed purchase or installation. The ESP32 firmware has not been user-tested.**
See docs/DEVICE_SIDE_POWER_BACKPLANE.md, docs/USB_SOLAR_MULTIPACK_POWER_ARCHITECTURE.md, docs/18650_MULTICELL_POWER_SIZING.md.

## Decided architecture
- Each removable serviceable cartridge has four matching 18650 lithium-ion cells in **1S4P** (nominal 3.6/3.7V, about 12Ah with four 3Ah cells), proper 1S protection and thermal monitoring.
- Every cartridge independently charges by USB-C when OUTSIDE device, or in cradle, and charges while INSTALLED through its charging-input contacts; one pack-specific charger circuit should serve all modes through source selection/protection.
- Each pack exposes separately protected regulated 5V POWER OUTPUT. The device does NOT combine raw 3.7V battery cell terminals or build another 4S/4P bank out of the cartridges. It uses a reverse-blocked power MUX/ideal-diode OR on separate pack output paths.
- Remote station: TWO slots. Handheld: TWO initially, potentially FOUR. Works with any one pack that can support peak load, and hot-swaps entire cartridges with no device reboot.
- Both devices get direct wall/USB-C power and optional solar. External source runs load with priority and leftover controlled input power charges each inserted cartridge; solar gets an appropriate input regulator/controller, source arbitration and monitoring. The battery-output combiner is NOT the charging input.
- The main system bus is protected **5V**, plus 3.3V where required. No 5V boost needed after combining correctly regulated 5V pack outputs.
- Report each pack's installed state, charge, health if genuinely measurable, time-to-empty estimate, source and fault conditions to handheld/web/Android and remote mesh.

## Hardware and module status (NOT ordered/verified)
| Location | Item | Per cartridge | Remote unit | Handheld (2/4 bays) | Status |
| --- | --- | --- | --- | --- | --- |
| Pack | Four genuine matched 18650s | 4 cells | 8 cells | 8/16 cells | CELL SKU TBD |
| Pack | Protected 1S4P holder/cartridge, pack BMS/fuse/temp sensor | 1 | 2 | 2/4 | NEED SELECTED & VALIDATED |
| Pack | 1S pack charger with **genuine load-sharing** and stable 5V output, ideally USB-C and solar | 1 | 2 | 2/4 | **Waveshare Solar Power Manager (D) CANDIDATE**, no tests |
| Pack | Dock rail/shroud, rated exposed output + charge input contacts (or mechanically guided USB-C charge connector), pack ID optional | 1 | 2 | 2/4 | CONNECTOR/MECHANICAL DRAWING TBD |
| Device | Protected cartridge socket; per-slot current limiting/eFuse, hot-insert and reverse blocking, input/output charge isolation | — | 2 | 2/4 | NEED DESIGN |
| Device | 5V pack-output combiner (TPS2121 dual MUX candidate) | — | 1 dual-input | four-source MUX/OR or rated cascades | NEED BOARD/SCHEMATIC |
| Device | External USB-C power input path, PD sink if beyond default 5V capability, fuse/surge/short protection | — | 1 | 1 | NEED DESIGN |
| Device | Solar input control (only if desired), 6–24V-compatible controller/source arbitration, input current limiting | — | optional 1 | optional 1 | NEED DESIGN |
| Device | External vs battery priority power MUX, stable output filters/hold-up sized by real load | — | 1 | 1 | NEED DESIGN/TEST |
| Device | Independent power delivery to installed cartridge CHARGE INPUTS, allocated current, no cross-charge | — | 2 channels | 2/4 channels | NEED DESIGN |
| Device | Fused 5V distribution, 3.3V regulator as necessary, per-pack voltage/current/temp/SoC health reporting | — | 1 supply + sensors | 1 supply + sensors | NEED DESIGN |
| Charger stand | Desktop dock with protected 5V supply per cartridge and input-power budget; works with each cartridge's own charger | — | optional 2 sockets | optional 2/4 | NEED DESIGN |
| CAD | Removable cartridge and device/bay enclosure, recess/rail/latch, access to USB, vent/heat/isolation from RF | — | 1 chassis | 1 chassis | DIMENSIONS TBD |

## Candidate source evidence, and blockers
- Manufacturer for Waveshare Solar Power Manager **(D)** explicitly advertises 3.7V rechargeable Li battery, Type-C and solar 6–24V inputs, simultaneous charging/discharging, regulated screw-terminal 5V/3A output: https://www.waveshare.com/solar-power-manager-d.htm ; https://www.waveshare.com/wiki/Solar_Power_Manager_%28D%29 .
- Its USB-C is specified for 5V **input**, even if output protocol advertises PD/QC. Do not claim PD charging from 9V/12V input or 5A battery charging. Wiki says "fast charging and discharging" simultaneously not supported; charger current and pack-size suitability must be verified.
- Critically, a separate **5V charger-input on slide-in docking contacts is not yet confirmed** for the candidate. USB-C connector handles 5V charge input, 5V screw terminal is an OUTPUT, and solar terminals require 6–24V; NEVER feed charger voltage backwards into output terminal, or 5V into a 6V-minimum solar terminal. Could mechanically mate USB-C in slot, use manufacturer-sanctioned input pads, or change board.
- Datasheet TPS2121 has TWO inputs, ~4A path rating and 5 microsecond typical MUX transition, but that does not guarantee other source can supply continuously, and its board must suit continuous/peak rail load: https://www.ti.com/product/TPS2121 .
- Standard IP5328P usually shuts off boost output when charging; this conflicts with guaranteed in-device seamless backup, so it remains a LESS preferred candidate. Never assume external power loss auto-enables sleeping charger board instantly.
- Pack rated 5V/3A OUTPUT current is NOT lithium CHARGING CURRENT. A single protected 1S4P pack must feed entire peak system while one pack remains; no assumption that multiple ORed outputs add their amp ratings equally.
- Same physical cartridge's pack cell service OFF device only; simple matched direct parallel cells cannot be freely changed with differing voltages/age. Protected complete pack fabrication and thorough thermal/electrical validation are mandatory.
- Waveshare ESP32-S3-Touch-LCD-7 own PH2.0 battery port is **not** compatible with the user-specified multi-pack charging system. Power the screen via its known correct **regulated USB-C 5V supply input**, design against PC USB backfeed: https://docs.waveshare.com/ESP32-S3-Touch-LCD-7 .

## Next proof-of-function milestones
P0. **No big purchases yet**: verify exact charging board pinout and whether a separate 5V USB-C charge path + continuous regulated 5V output can work on the candidate board with a protected 1S pack; confirm tested panel/adapter transition. Check load current with screen/radar and remote ESP32 measurements.
P1. First battery cartridge: bench one candidate charger with properly selected/protected genuine cell(s) and current-limited test supply, monitor 5V output, USB input, heat, charge behavior, transient response and failure modes. Test using one cell/pack under safe manufacturer's ratings before committing four-cell pack.
P2. First remote power backplane: two current-limited emulated regulated 5V cartridge outputs -> individually protected dual-source selector -> measured device 5V load. Test remove A, B, low input, source changes, no reverse current, startup/boot and brownout.
P3. Install two qualified complete protected cartridges, verify hot swap with on-device charging enabled and disabled, independently charge via cartridge USB-C and cradle, verify input source does not backfeed outputs. External USB directly serves load while surplus charges each docked cartridge.
P4. Integrate optional solar with controlled sunlight-equivalent power and input selection; measure under clouds, supply loss, charging throttle, pack source recovery, hot swap, outdoor thermal protection.
P5. Reuse validated subcircuits for 2->4 handheld bay expansion with main screen full-brightness and multiple radar/wireless peak power. Review total mass, thermal and safety.
Every stage requires precise measured pass/fail logs, current/voltage, connector temperatures and saved checkpoints. No firmware baseline changed by this document.

## Proposed first-purchase subset only after P0 compatibility review
- ONE manufacturer-identified Waveshare Solar Power Manager (D) candidate; ONE correctly selected genuine protected 18650 pack/test cell; a multimeter and current-limited power supply if not already available; test load capable of verifying 5V/3A output.
- ONE verified TPS2121 power-MUX breakout **or** separate appropriately rated dual input selector; test with two regulated supply simulators first.
- Prototype mating connector/contacts AFTER actual physical 5V charge input and output mapping is validated. Do not buy bulk 4x boards/cartridge housings before P1/P2.

## Purchase preference
User prefers Amazon when possible; source manufacturer pages for engineering verification, and link to exact confirmed Amazon item (not unverified/search placeholders) when shopping becomes actionable.
