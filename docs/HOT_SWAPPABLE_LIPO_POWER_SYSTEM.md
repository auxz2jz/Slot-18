# Modular Hot-Swappable LiPo Battery Cartridges — Design Requirements

Date: 2026-10-07. STATUS: PLANNED / RESEARCH ONLY. No battery bay, charger, PCB, CAD, firmware or runtime/charging safety test has been implemented or user verified. No modifications to existing untested firmware. Main screen currently planned to be powered via regulated 5V USB wall adapter for hardware bench tests.

## Explicit user requirements
- Battery chemistry/form: **flat silver pouch 1S lithium-polymer (LiPo) cells**, typically gray foil and orange/yellow Kapton tape. Not cylindrical 18650/21700, and no bulky 12V battery for final handheld/stations.
- **Remote stations:** two independent removable battery bays; operate on either one, or both installed together; maintain operation when either is removed with the other adequately charged.
- **Main handheld:** four independent removable battery bays potentially located near case corners; operate on ANY one, two, three or four packs if each is rated to sustain entire load. Extend runtime with extra packs; allow exchanging one pack without shutting down.
- Prefer **3D printed slide-in battery cartridges** with contact set at insertion end; identical safe cassette geometry where feasible between remote and handheld; avoid unplugging loose JST leads repeatedly.
- Display/transmit per-bay installed/missing/charging/discharging, battery % state of charge (SoC), voltage, estimated remaining time, degraded/aging state of health (SoH when actually available), cartridge ID, overall energy and estimated runtime, low battery alerts, safe-to-remove alerts.
- Remote sensor status including per-bay charge state and estimated time must be available over existing planned Wi-Fi/ESP mesh to main 7-inch screen, Android app and web client.

## Important electrical architecture (concept; not a wiring diagram)
Each LiPo battery cartridge is one protected 1S LiPo pack with short-circuit/overcurrent/overcharge/overdischarge protection, insulation and ideally a temperature thermistor, matched connector rating and physically keyed shrouded contacts. Where practical, attach an ID/gauge in the cassette so learned health data follows the pack.

Pack A -> protection + supervised hot-swap connector -> isolated source power-path
Pack B -> protection + supervised hot-swap connector -> isolated source power-path
(Optional C,D handled identically, one independent path each)
Isolated source rails -> **engineered multi-input ideal-diode OR / power-selection stage** -> load-capable regulated 5V bus -> ESP32 touchscreen/remote station and radars.

Do not directly wire raw 1S LiPo packs in parallel or series. Do not assume that stacking generic 2-input breakout modules works without checking reverse paths and current limits. 'Use both' means approved, isolated source management, NOT uncontrolled direct cell paralleling or guaranteed equal current sharing. The system must remain functional on a single chosen pack at worst-case full device load; validate peak current and handoff on bench.

Candidate power-path components:
- Analog Devices LTC4412 low-loss ideal-diode controller + external MOSFET, explicitly supports multiple-battery source OR'ing: https://www.analog.com/en/products/LTC4412.html . It is a controller IC, not a complete turnkey four-bay board.
- Texas Instruments TPS2121 dual-input power multiplexer (max ~4A per input depending on implementation), seamless input handoff: https://www.ti.com/product/TPS2121 . A two-input part, NOT an all-in-one four LiPo charger/hot swap PCB.
- Another option is a per-bay approved buck/boost regulator feeding a properly engineered 5V ideal-OR stage; select based on peak current/efficiency and whether lower-voltage cartridges can supply full load. No selection should be finalized before real current measurements.
- Hold-up capacitors, overvoltage/current protections, per-bay current detection, insertion/debounce sequencing, hot-plug inrush and voltage dip handling need engineering proof. The MCU firmware must never be the *only* mechanism keeping source power safe; hardware must prevent reverse current and brownout.

## Charging options
Option A (RECOMMENDED FIRST): **removable battery cassettes charged in an external dock**, independent 1S cell charger channel for each bay. This keeps handheld/remotes lighter and avoids packing up to four active charging circuits into a tight enclosure. It is acceptable to charge one or multiple at a time when separate channels and correctly rated supplies are used. External dock uses a charging-enable contact, thermal monitor, charge state and per-slot fault protections.

Option B (LATER): independent charger + power-path manager per battery slot built into station/handheld. Safe onboard charging while mains/USB is connected must use managed input current and appropriate thermistors; charger circuits per pack, not one charger tied directly to 2–4 cells. Four channels plus active load require sufficient power and heat dissipation. The Waveshare screen's stock PH2.0/CS8501 580mA charge circuit is **single 3.7V cell only**, recommended <=2000mAh; DO NOT wire the four cartridges to PH2.0. Power display from protected regulated **5V external USB input**, keep native battery input disconnected unless an explicitly validated alternative is chosen.

Example single-bay educational charger/load-share board: Adafruit BQ24074 USB/DC/Solar charger product 4755 supports one 1S LiPo, USB-C and LOAD sharing; board max LOAD 1.5A and OUTPUT ~3–4.4V, so it's not a 5V ESP32 screen supply and not an automatic 2/4-way swap controller. https://learn.adafruit.com/adafruit-bq24074-universal-usb-dc-solar-charger-breakout/overview . It is a building block, not a complete solution.

Safety requirements: correct charge voltage (typically 4.2V for standard 1S LiPo; check pack variant), limited charge current matched to cell capacity, temperature sensing/inhibit outside safe range, no charging swollen/damaged pouches, fused/limited short-circuit paths, protected unpowered external contacts, no exposed energized terminals, keying/reverse polarity control and mechanical enclosure with pouch puncture/squeeze/swelling relief. Printed PLA/PETG enclosure is not a certified fire containment device. Do not put magnets close to future GNSS-compass.

## Battery reporting and fuel gauging
- For each installed slot, request: pack_present, pack_id when known, pack_health/fault, SoC_percent, voltage_mV, temperature_C where thermistor supported, current_mA when measured, remaining_mAh / remaining_Wh where estimated, status (charging/discharging/idle), source_active, estimated_time_remaining_min, and confidence/quality of estimate.
- MAX17048 is inexpensive 1S I2C cell voltage/SoC gauge, **not a full independently accurate aging/SoH or runtime estimator**; default I2C address 0x36, and if using multiple same-address gauges on one bus use suitable I2C multiplexing and verify Waveshare board's existing I2C addresses/voltage. https://learn.adafruit.com/adafruit-max17048-lipoly-liion-fuel-gauge-and-battery-monitor/pinouts
- TI BQ27441-G1 1S Li-ion/LiPo Impedance Track gauge reports remaining capacity, SoC and estimates SoH; supports removable packs when configured appropriately. https://www.ti.com/product/BQ27441-G1 . MAX17055 is another more sophisticated option with time-to-empty, dependent on hardware and calibration.
- Prefer pack-carried gauge/ID if removed cartridges should retain learned SoH and be reliably identified when docked elsewhere; a system-side gauge per slot is cheaper but hot insertion/resets require careful configuration and learning management.
- Runtime estimate: aggregate **usable remaining Wh** across individually connected packs (account for converter efficiency, reserve, current and thermal limits), then estimate against measured recent average device W, updating over time. Display 'calculating' or broad range while load changes, with explicit "only X bay(s) installed". Battery percent alone does not reliably predict runtime or long-term health.
- Degraded packs may have normal open-circuit voltage but reduced effective capacity/high resistance. Use gauging history to show SoH as separate from remaining SoC. Each remote station can include per-pack telemetry with its normal sensor packets (no special permanent cloud or GPS requirement).
- Low-battery warnings well before total collapse and 'Safe to remove B' only when another healthy pack can meet measured peak current. If no safe fallback, ask for replacement pack insertion BEFORE allowing removal. Implement undervoltage safe restart and recovery state.

## Slide-in cartridge connector ideas (not yet selected)
1. **Recessed gold-plated spring contacts / pogo pin docking contacts** against plated PCB pads or nickel/gold plates, supported by keyed 3D printed rails and latch. Best user-feel, but check per-contact rated continuous/peak amps, resistance, wear cycles, hot-mate arcing, pin wipe, polarity/shrouding, no bridging. Prefer 4+ contacts when thermistor / identity needed, with current-carrying pins sized for worst single-pack input current.
2. **Guided blind-mate battery connectors** from qualified connector families (e.g. Molex Micro-Fit or specialist blind-mate battery connectors) with real mechanical drawings, wear cycles, current limits. They may work better than pogo pins but still need floating alignment or slide-in mechanical guides; not all Micro-Fit parts are blind-mate compatible.
3. **XT30-type keyed connectors** are electrically sturdy for battery current but not necessarily suitable for repeated unguided blind mating; only use mounted/guide-aligned matching connector parts, never hot-swap bare exposed connector leads.

Create cartridges so battery itself is never physically scraped against spring contacts or housing corners. Replaceable cassette outer shell is insulated, strain-relieved, with recessed terminal and removal pull-tab and easy polarity marker. Plan four modules around screen corners only after checking screen rear PCB, connector clearance, mass balance and grip; exact 3D-print dimensions cannot be fixed until user chooses physical LiPo SKU and charger/connector family.

## Amazon-only comparison searches (not independently verified listings)
- Protected 1S LiPo pouch 3.7V 2000–3000mAh: https://www.amazon.com/s?k=3.7V+2000mAh+LiPo+battery+protected+pouch
- Protected 1S LiPo pouch 3.7V 4000mAh: https://www.amazon.com/s?k=3.7V+4000mAh+LiPo+battery+with+protection
- Pogo battery docking pin contacts, look for adequate **actual continuous current rating**: https://www.amazon.com/s?k=high+current+pogo+pin+battery+connector+spring+loaded+contacts
- Pogo pin PCB pads / module pair: https://www.amazon.com/s?k=pogo+pin+docking+connector+4+pin+battery
- Molex Micro-Fit 3.0 connector kit: https://www.amazon.com/s?k=Molex+Micro-Fit+3.0+connector+kit
- XT30PW PCB panel-mount connectors: https://www.amazon.com/s?k=XT30PW+PCB+connector
- BQ24074 1S charging/load-share single channel: https://www.amazon.com/s?k=Adafruit+BQ24074+USB+C+LiPo+charger
- BQ27441 single-cell fuel gauge module: https://www.amazon.com/s?k=BQ27441+fuel+gauge+module
- MAX17048 LiPo battery gauge breakout: https://www.amazon.com/s?k=MAX17048+battery+fuel+gauge
- MAX17055 advanced fuel gauge: https://www.amazon.com/s?k=MAX17055+fuel+gauge+module
- TCA9548A I2C multiplexer to read multiple equal-address sensors: https://www.amazon.com/s?k=TCA9548A+I2C+multiplexer
- TPS2121 seamless 2-source power mux board: https://www.amazon.com/s?k=TPS2121+power+mux+board
- LTC4412 ideal diode battery power path: https://www.amazon.com/s?k=LTC4412+ideal+diode+board
- 1S separate-channel multi-bay lithium charger: https://www.amazon.com/s?k=independent+2+channel+1S+LiPo+charger and https://www.amazon.com/s?k=4+channel+1S+LiPo+charger
Verify advertised capacity, dimensions, cable polarity, protection/thermistor circuitry and especially electrical pinout against actual purchased model BEFORE wiring.

## Phased work and tests
1. Keep current main ESP32 on approved regulated wall USB for v0.2.0 display and one-radar tests. No actual battery hot swapping yet.
2. One protected 1S pouch pack + appropriate power converter + a single battery slot test. Measure continuous/peak load and temperature; confirm voltage monitor, SoC, shutdown.
3. Remote TWO slot independent protection/OR circuit bench-tested with calibrated supply/electronic load first, then genuine protected packs. Verify one pack, both, insertion/removal under load, brownouts and reverse current. Only after electrical safety verified print fitted cartridges.
4. Handheld FOUR slot expansion after 2-slot proven: any of the four alone and all combinations under real peak screen/radio/radar load; wall USB takeover; pack temperature and charging dock separate tests.
5. Train/cross-check SoC/SoH estimates with measured discharge time and changing workloads; publish sensor station packet with pack/bay telemetry to main, later Android/web.
6. Record diagnostic event/request/state/result, dedicated guided test checklist, user-observed verification and rollback checkpoint per Master Instruction Library. No earlier software/physical baseline overwritten.
