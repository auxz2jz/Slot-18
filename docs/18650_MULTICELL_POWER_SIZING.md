# Proposed Multi-18650 Battery-Cartridge Sizing — Under Evaluation

Date: 2026-10-07 (user time). No batteries selected or physically tested. The final preference for 18650 versus pouch is under discussion. Preserve existing pouch-cell proposal as an alternative. The v0.2.0 ESP32 firmware and physical display remain unverified; testing main unit on a regulated 5V USB wall adapter remains the current plan.

## Requirement interpretation
User is considering **FOUR individual 18650 batteries on EACH SIDE** of every remote sensor station, potentially in two independent removable 4-cell cartridges (eight cells total). The handheld may use one or more similar cartridges, up to four cartridges, with easy battery replacement and no total loss of power. The user wants longer unattended runtime and inexpensive ready-made holders, 3D-printable outer housing, battery health telemetry, and swappable cells.

## Power budget — estimates pending hardware instrumentation
- A Hi-Link LD2450 module: published 5V 120mA average (~0.6W). Source: https://manuals.plus/m/374d7b6608cbce20e803b3aa4f405637826960c1ed42a858dedfaa97a3253478
- ESP32-S3 Wi-Fi radio itself receives at ~88-91mA (at 3.3V), and can have high transmit peaks; whole-board current including CPU/regulators/mesh is higher and variable. Source: https://documentation.espressif.com/esp32_s3_datasheet_en.pdf
- Remote planning ranges: ~0.8-1.5 W one-radar light reporting; ~1.5-2.5 W continuous mesh relay with several sensor/peripheral loads; potentially >=2.5-4 W with more active sensors. NOT measurements.
- Waveshare 7in ESP32-S3 screen is documented around 5V 450mA (~2.25W) by itself, not including radar or wireless overhead: https://docs.waveshare.com/ESP32-S3-Touch-LCD-7
- Handheld planning ranges: ~3.5–6 W with screen on and wireless/radars, more if additional continuous peripherals or bright backlight. NOT measurements.

## Calculating battery capacity
Single typical quality 18650 cell at 3000mAh * 3.6V nominal = 10.8 Wh nominal; effective usable energy assumed ~80% after boost/regulated conversion, load limits and reserve = 8.64 Wh per cell. Actual varies with model, age, temperature, depth of discharge and protection cutoffs.

| Cell count | Nominal Wh | Usable assumption Wh | Remote at 1.5W | Remote at 2.5W | Handheld at 4W | Handheld at 6W |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 4 | 43.2 | 34.6 | 23h | 14h | 8.6h | 5.8h |
| 8 | 86.4 | 69.1 | 46h | 28h | 17.3h | 11.5h |
| 12 | 129.6 | 103.7 | 69h | 41h | 25.9h | 17.3h |
| 16 | 172.8 | 138.2 | 92h | 55h | 34.6h | 23.0h |

An 8-cell continuous mesh relay station is probably 1–2 days at typical proposed loads, not a multiweek unattended deployment. Larger or solar/top-up options require load measurements. Cell mass is often ~45–50g each, so eight cells ~360–400g not counting holder, casing, PCB and wiring; 16 cells ~720–800g by themselves and may be unwieldy handheld.

## Battery topology and serviceability
IMPORTANT: **ready-made holders marked 1S4P / PARALLEL tie all four cells directly together. Inserting a charged cell alongside discharged cells can cause unsafe equalization currents. Such holders do NOT make independent cells electrically hot-swappable.** A failed cell could be fed by peers. Mixing brands/ages/state-of-charge is hazardous. Do not use these as unattended user-serviced hot swap systems merely by adding a 3D printed cover.

Best first architecture (to be engineered/validated):
- Remote station: TWO complete **protected four-cell cartridges**, each manufactured/matched 1S4P or suitable alternative pack architecture with correct pack-level BMS/fuses, under dedicated reverse-current-blocked input, rated boost/power output. **Hot-swap entire cartridge**, not individual cells while pack is live.
- Handheld: start with 1 or 2 matching four-cell cartridges and expand only if mass/runtimes require, rather than immediately installing 4x4=16 cells.
- Under any scheme, each cartridge on its own must support continuous AND peak current of whole unit. Regulated 5V input for Waveshare is separate from the vendor single-cell PH2.0 charger.
- If genuine **single-cell hot-swap** is a firm requirement, each 18650 must have its own protected/monitored power input with controlled insertion/inrush/backflow blocking/isolated power path, then shared regulated power. This is far more complicated/costly than simple 1S4P holders; don't claim a generic parallel holder satisfies it.
- Series packs (4S, 2S2P) require corresponding series BMS/balancing and cannot have one cell removed while powered. Battery pack uses multiple series cells (not four separately removable hot swap sources).
- The independent cartridge system preserves pack identification, SoC, SoH if sensorized, current, voltage, temperature, source-active/standby, last seen, and time-to-empty; the web/Android/7in panel must show estimated rather than guaranteed hours.
- For unattended Li-ion, temperature, short/overcharge/overdischarge protection, mechanical retention, water ingress, safe charging, and genuine correctly rated cells are necessary; pre-made holder spring clips can have unreliable contacts under vibration and may not fit extra-length protected cells.

## Amazon-only searches, not verified compatible products
- Four-slot PARALLEL 1S4P 18650 holder (NOT safe independent hot-swap on its own): https://www.amazon.com/s?k=4+18650+parallel+battery+holder+1S4P
- Four-slot SERIES 4S 18650 holder (different high-voltage topology): https://www.amazon.com/s?k=4S+18650+battery+holder
- Protected 18650 ~3000–3500mAh cell: https://www.amazon.com/s?k=protected+18650+3500mAh+battery
- Individually charging 18650 cells in a standalone independent 4-bay charger: https://www.amazon.com/s?k=XTAR+VC4+charger
- Ready-made protected Li-ion battery module with BMS, regulated 5V output and serviceable enclosure, to evaluate but not assume interchangeable: https://www.amazon.com/s?k=4x18650+5V+power+bank+module+protection
- Independent input ideal diode or power MUX: https://www.amazon.com/s?k=ideal+diode+MOSFET+power+ORing+module
- Independent battery fuel gauge (per cartridge) to report battery health: https://www.amazon.com/s?k=BQ27441+battery+fuel+gauge

## Next design actions
1. Decide whether 'hot swap' applies to **complete 4-cell cartridges only**, or individually to 18650 cells in each pack. Both remain in discussion.
2. Choose expected unattended run duration target and full station sensor set (one LD2450 vs 3x radar plus GNSS/thermal), plus whether mesh relay must listen 24/7.
3. After one physical remote prototype, measure current in idle, active detection, frequent mesh relay, reconnection and transients at regulated input. Recalculate expected runtime using measured Wh and load, and add a safety reserve.
4. Select known genuine 18650 protected battery models and fit-tested contact holders; confirm pack fusing/BMS and physical charging design under professional electrical safety review.
5. Design and print removable case rails and cassette with room for board, radar window and battery telemetry; no pack CAD declared final before physical battery SKU and connectors selected.
