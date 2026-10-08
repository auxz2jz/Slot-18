# Multi-cartridge 18650 power system: USB-C, solar, live charging and hot-swap

Recorded: 2026-10-07 America/Los_Angeles. STATUS: **REQUIREMENT CONFIRMED; CIRCUIT TOPOLOGY PLANNED, NOT BUILT OR TESTED**. This supersedes the earlier assumption that individual cells would be changed while installed. No firmware, battery packs, PCB schematic or printed enclosure validated. Existing wall-USB-powered ESP32-S3 bring-up remains first actual device test.

## Confirmed user operation
- **Swap only entire removable cartridges during device operation**. The electronics must keep the remote station or handheld powered with at least one other adequately charged cartridge OR with external power.
- Each remote station: **two removable cartridges, nominally four 18650 cells in each** (eight total), subject to final pack dimensions/current measurements. Main handheld: **two removable four-cell cartridges initially, potential expansion up to four cartridges** (8 to 16 cells), subject to weight/clearance and actual load.
- A detached cartridge is user-openable so individual 18650 cells can be inspected, replaced and charged after it has been taken out. It is *not* allowed to service bare individual cells while cartridge is inserted.
- Both handheld and remotes get an **external power/charging port** which should power the device directly and simultaneously charge installed packs *when the external source has enough capacity*. No internet needed.
- Both types support **optional solar panel input**, providing live system load first and using surplus PV energy to recharge cartridges if conditions permit.
- Automatic input preference: appropriately sized USB/wall adapter when available, then solar input where appropriate, then batteries when external power is unavailable or insufficient; do not inadvertently connect wall input and solar together without engineered input-selection/backfeed protection.
- Preserve **per-cartridge** SoC, actual measured voltage/current/temperature where instrumented, fault, charge/discharge/idle/active, aging/SoH where a capable gauge is fitted, and estimated minutes left (estimated, not guaranteed). Send each remote's status to main ESP32 and later Android/web; show one vs two present, and external USB/solar source active.
- Cartridge outer shell will be 3D-printed with recessed keyed, appropriately rated blind-mate/docking connector, mechanical retention and protected exposed contacts. Use one cartridge footprint when practical across remotes/handheld.

## Important design clarification: cells INSIDE cartridges
Option A (cost/size preference, evaluate professionally): matched 4-cell 1S4P pack in removable holder/cartridge with properly chosen protection, current/temperature monitoring and **one correctly sized 1S charger per cartridge**. The 4 cells share the same parallel voltage. A discharged new cell dropped into a partially charged parallel set creates potentially severe equalization current. To service, first remove cartridge, disconnect/power down, assess health/voltage/capacity of ALL cells, use matched cells; in practice **replace the matched set** if one is damaged or markedly degraded. Do not imply arbitrary one-cell replacement is safe just because cartridge is off-device.
Option B (higher cost but true independent per-cell service): four isolated/protected single-cell channels inside each cartridge, with per-cell charging/health supervision and engineered discharge OR paths. Individual cells may then be changed offline without hard-paralleling unequal voltages; validate inrush, one-channel power load, reverse current and thermal dissipation. This may be preferable if selective single-cell replacement is an important requirement, but adds hardware size/cost and needs a real electrical design.
Option C: 4S series pack plus 4S balancing BMS and 4S charger; lower external current but breaking the series pack is potentially risky and all cells need balancing/matching. Not selected by default. BQ25798 '1–4-cell' spec means up to **four SERIES cells** per charger, not four independent packs; cannot connect two/4 cartridges to one battery terminal and call it isolated charging.

## Functional block arrangement (NOT final schematic)
INPUT SIDE:
USB-C (power role/PD negotiated if >5V requested, reverse-feed blocking, overcurrent and USB plug protection)
           \
            --> input selector / source protection --> input power budget/distribution
           /
Optional appropriately rated photovoltaic panel --> solar controller/MPPT or input voltage regulation --> same source selector

LOAD SIDE:
External selected input --> protected system power path --> regulated **5V** rail --> ESP32 touchscreen/sensors
                                                     ^
Cartridge A --> its own BMS/protection + gauge + isolated load path -------|
Cartridge B --> its own BMS/protection + gauge + isolated load path -------|
Handheld Cartridge C/D (when built) identically independent ------------|

CHARGE SIDE:
Input distribution --> **independently controlled, correctly rated Li-ion CC/CV charger for cartridge A** --> its own pack
Input distribution --> **separate charger for cartridge B** --> its own pack
Input distribution --> separate chargers for C/D when added, subject to input/current limits
A charger must never be tied directly across two unrelated removable cartridges; a single-channel charger for one multi-cell *pack* is not by itself a multi-pack hot-swap charger.
A properly selected charging/power-path IC may integrate some input/mux/load/charging functions; still must engineer the independent multiple pack channels, common 5V output and safe hot insertion/removal.
Input current management must reserve enough power for LOAD before enabling/scheduling battery charge, and fall back safely to battery if solar drops at dusk/shade or USB is unplugged. Charger should suspend/reduce current if panel/source cannot supply both. Partial clouds and low solar may produce zero surplus charging.

## Components worth evaluating — NOT yet a chosen complete board
- **TI BQ25798** charger IC / BQ25798EVM: supports 1–4 Li-ion series cells, up to 5A charge, USB/DC solar input with MPPT-style VOC tracking, power-path, backup switchover, input selection and battery temperature protection. https://www.ti.com/product/BQ25798 ; https://www.ti.com/tool/BQ25798EVM . **ONE charger IC normally manages ONE pack; two cartridges need separate charger/protection paths plus engineered power sharing.** Vendor EVM is a development board, not plug-and-play multi-cartridge controller or bare 5V load supply.
- **Adafruit BQ24074 USB/DC/Solar charger** example: for a *single 1S Li-ion/LiPo pack*, 5–10V input, power-path load sharing up to 1.5A; OUT varies around 3–4.4V, not regulated 5V; requires separate 5V conversion and per-pack separation. https://learn.adafruit.com/adafruit-bq24074-universal-usb-dc-solar-charger-breakout . Demonstration building block, NOT two-pack or 4-pack independent charger. It is not true MPPT, uses input voltage control to prevent solar panel voltage collapse.
- Potential independent pack load sharing: verified correctly rated power MUX / ideal-diode OR with MOSFETs, inrush/current limits, backfeed prevention; charging paths must remain independent and not be accidentally blocked by the discharge ideal diode.
- USB-C PD controller/negotiation may be required for charging multiple packs at appreciable rates: 5V/3A input is only ~15W total before losses, potentially insufficient for full-brightness display + radars + 2–4 chargers. A PD-capable **charger-side** 20V supply, when properly negotiated/rated, can provide more input power; never apply PD 9/12/20V directly to the Waveshare's 5V input.
- Use per-pack rated protection/BMS, fusing or current limit, thermal sensors, genuine current-rated recessed mating connectors; important hot-unplug under load transient is a hardware test, not a software feature.
- For external solar, use solar-specific MPPT / panel voltage regulation and properly selected PV voltage/current panel, not a bare panel directly to 18650s or a cheap standard TP4056 charging module unless panel regulated and battery topology protected. Panels and Li-ion cells should not share a sun-heated sealed container; battery must remain in safe charge temperature range.

## Example power budget and panel size — illustrative, NEVER promised runtime
Using 8 x quality 3000mAh 18650s at 3.6V nominal = 86.4Wh nominal. With 80% assumed effective usable energy -> about 69Wh; around 34h at average 2W station load, 28h at 2.5W. Actual measurement and converter/cell/BMS behavior controls.
For a continuously operating 2W remote station: daily load = 48Wh/day. A nominal 20W panel with an illustration of 4 equivalent full-sun hours/day and 70% net capture gives ~56Wh/day, leaving only ~8Wh/day for recharge; cloud, winter, shading and poor orientation can make actual capture much less. A **20–40W solar panel** may be reasonable to evaluate for a 1–2.5W station, but the true size needs site/season measured sun-hours and desired recovery rate. A nominal 30W panel under these same example assumptions gives ~84Wh/day, ~36Wh available to recharge after a 2W load; not guaranteed. Main handheld may average ~4–6W with screen/radar; a pocket-sized integrated panel is unlikely to keep it operating and recharge rapidly; an optional external foldable 20–60W panel should be evaluated, with operation schedule and actual draw.
All calculations omit unusual cold/heat behavior and extreme weather. For remote multi-day unattended deployment, include required energy reserve for consecutive poor-sun days and battery low-voltage recovery.

## Connector/enclosure requirements
- Same pack-side docking connector for remote and handheld if practical, with power+, power return, pack-present/ID, optional thermistor/data pins. Recessed and keyed connector. Do not short high-energy packs if metal touches contacts.
- Input connections: splash-protected USB-C or rated DC input; optional weather-resistant solar connector, strain relief, polarity protection, surge/ESD; appropriate enclosure ventilation/cable glands without shading or covering radar antenna.
- Design for *full four-cell pack* as removable module, not individual cell live insertion. Keep rear monitor mount supports and screen cables clear.
- Provide possible per-cartridge charging status LEDs, safe-to-eject feedback and charge suspend when inserted pack overheats. One pack left must independently supply the entire continuous + peak device input power if USB/solar is unavailable.
- Indoor tests continue on wall USB first. Remote outdoor charging and waterproofing are later verified stages, not present behavior.

## Amazon-only model search links — availability, electrical ratings unverified
- 4-cell 18650 holder with cover: https://www.amazon.com/s?k=4+18650+battery+holder+with+cover
- Protected 18650 reputable cells: https://www.amazon.com/s?k=protected+18650+battery+3500mAh
- 18650 pack BMS protection 1S or 4S (MUST match chosen topology): https://www.amazon.com/s?k=1S+18650+BMS+board
- USB-C solar Li-ion charging boards: https://www.amazon.com/s?k=USB+C+solar+Li+ion+battery+charger+power+path
- BQ25798 solar charging modules / EVM: https://www.amazon.com/s?k=BQ25798+battery+charger+module
- Properly rated independent multiple-input power path: https://www.amazon.com/s?k=ideal+diode+battery+power+ORing+module
- 20–40W solar panel for remote stations: https://www.amazon.com/s?k=30W+solar+panel+portable+waterproof
- Foldable handheld solar panel 20–60W: https://www.amazon.com/s?k=40W+foldable+solar+panel
- IP-rated USB-C panel connectors: https://www.amazon.com/s?k=waterproof+USB+C+panel+mount+connector
No third-party shopping sites are recommended; engineering manufacturer spec URLs above are included solely for accuracy.

## Required test/diagnostic plan
1. Screen bring-up v0.2.0 remains the current user test; firmware untouched. Measure screen+radar peak current to size 5V rail.
2. Prototype independent protected cartridge source switching **with benchtop emulated batteries/current limits**, not a self-wired pile of unknown 18650 cells. Confirm any one source alone, two sources, load step/transients and safe removal under external power and batteries.
3. One supervised Li-ion pack charging branch: wall USB powering load + charging pack; then pause input while safely switching to battery. Verify actual current, charge cutoff, thermal sensor/fault behavior. Do not depend on ESP32 software alone for voltage protection.
4. Two-pack charging concurrently from a validated input. Validate source current budget, no cross-charging, pack source ID, hot swap from A to B and insertion contact inrush. Repeat with sunlight fluctuation, panel disconnected, wall USB connected/disconnected.
5. Expand to 2/4 packs for handheld and test all needed combinations and charging schedules. UI displays accurate pack count, SoC/SoH if measured, source USB vs solar vs battery, low battery, power watts, runtime estimated/confidence and charging faults on main and remote Android/web telemetry.
6. Only record results as physically VERIFIED after user sees tested behavior. Save measurements/diagnostics to project checkpoint; maintain no edit of last-verified source.
