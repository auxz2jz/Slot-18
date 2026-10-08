# Hardware inventory, alternatives and purchasing references
These are candidate parts, not purchased/tested components. Amazon URLs are SEARCH links for matching exact model; price/stock changes.

## Base unit
- Waveshare ESP32-S3-Touch-LCD-7, capacitive touch, 800x480, ESP32-S3 N16R8 (16 MB flash, 8 MB PSRAM): https://www.amazon.com/s?k=Waveshare+ESP32-S3-Touch-LCD-7
- Manufacturer: https://docs.waveshare.com/ESP32-S3-Touch-LCD-7
- Manufacturer LVGL setup: https://docs.waveshare.com/docs/ESP32/ESP32-S3/ESP32-S3-Touch-LCD-7/Arduino/Arduino-LVGL-Demo
- UART terminal and USB-UART selected with jumper; not two guaranteed independent UARTs. Exposed I2C uses GPIO8/9 with shared touch/IO-expander devices. Do not assume pins are free.

## Initial radars — begin with one each
1. Hi-Link LD2450, ~24 GHz target tracking: up to three tracks with X/Y (local planar only), approx 6m advertised range. UART high baud; verify actual model data. https://www.amazon.com/s?k=HLK+LD2450
2. DFRobot C4001 SEN0609, 25m MOVING detection; stationary presence shorter (nominal 16m) and distance/speed, not precise XY. Manufacturer says UART 9600 and documents I2C 0x2A/0x2B; verify physical connector/pinout. https://www.amazon.com/s?k=DFRobot+SEN0609+C4001 ; https://wiki.dfrobot.com/sen0609/docs/20935
3. DFRobot C4002 SEN0691, near-stationary human presence, not XY target tracking. https://www.amazon.com/s?k=DFRobot+C4002+SEN0691 ; https://www.dfrobot.com/product-3081.html

## Candidate interfaces/controllers
- SC16IS752 I2C-to-two-UART bridge: https://www.amazon.com/s?k=SC16IS752+dual+UART+module . Do not put high-baud LD2450 on bridge without throughput proof; place on a confirmed hardware UART first.
- ESP32-S3 development boards for remote stations: https://www.amazon.com/s?k=ESP32-S3+DevKit+N16R8
- TCA9548A I2C channel switch for device-address clashes: https://www.amazon.com/s?k=TCA9548A+I2C+multiplexer
- RS485 transceivers or suitable long-cable interface if wired stations needed: https://www.amazon.com/s?k=isolated+RS485+module+3.3V

## Pose and location
- BN-880 GNSS + compass: https://www.amazon.com/s?k=Beitian+BN-880+GPS+compass (module variations exist; check interfaces & voltage).
- BNO085 9-axis IMU: https://www.amazon.com/s?k=BNO085+9DOF+IMU (requires magnetic/heading calibration and integration; IMU alone is not absolute compass accuracy).
- Optional u-blox ZED-F9P RTK-GNSS: https://www.amazon.com/s?k=ZED-F9P+RTK+GNSS+module (corrections/base station needed for cm-class performance).
- Fixed remote sensors may use manually surveyed map coordinates + measured orientation rather than powered GNSS.

## Thermal/obstacle/motion/other
- AMG8833 8x8 IR thermal: https://www.amazon.com/s?k=AMG8833+thermal+sensor
- MLX90640 32x24 IR thermal: https://www.amazon.com/s?k=MLX90640+thermal+camera+module
- RPLIDAR A1 360-degree 2D LiDAR for exposed wall/obstacle outlines: https://www.amazon.com/s?k=SLAMTEC+RPLIDAR+A1
- VL53L1X ToF narrow proximity: https://www.amazon.com/s?k=VL53L1X+distance+sensor
- VL53L5CX multizone depth: https://www.amazon.com/s?k=VL53L5CX+sensor
- LD2410C presence radar: https://www.amazon.com/s?k=HLK+LD2410C
- DFRobot C1001 60 GHz specialized indoor sleep/fall: https://www.amazon.com/s?k=DFRobot+C1001+SEN0623
- TI IWR6843ISK, advanced 3D point/track capable radar, nontrivial integration: https://www.amazon.com/s?k=IWR6843ISK
- PIR AM312 motion: https://www.amazon.com/s?k=AM312+PIR+sensor
- Ultrasonic JSN-SR04T: https://www.amazon.com/s?k=JSN-SR04T+ultrasonic+sensor
- Magnetic door reed switch: https://www.amazon.com/s?k=magnetic+reed+switch+module
- ADXL345 vibration/accelerometer: https://www.amazon.com/s?k=ADXL345+accelerometer+module
- ESP32-S3 OV2640 camera / alternate camera interface: https://www.amazon.com/s?k=ESP32+S3+camera+OV2640 (CPU and privacy implications)
- Environmental: BME280 temperature/humidity/pressure; SCD40 CO2; magnetic field/Hall sensor; air quality sensors. All optional; ensure appropriate power and interface.

## Power and supplies
### CURRENT TEST PHASE — MAIN SCREEN ON WALL POWER
- **User's confirmed plan (2026-10-07): plug main 7-inch Waveshare ESP32-S3 touchscreen into a household wall outlet using a correctly rated USB wall charger and USB cable.** Keep it wall-powered for initial firmware, screen, touch and sensor testing. Not a built-in wall/mains supply: never wire AC mains to ESP32 components.
- Candidate USB supply: regulated 5V wall adapter with enough continuous and peak current for screen + active sensor load; 5V/3A is a useful starting capacity **subject to verifying Waveshare input and attachments**: https://www.amazon.com/s?k=5V+3A+USB+C+wall+charger
- USB-C cable capable of power and data when flashing/debugging: https://www.amazon.com/s?k=USB+C+data+power+cable
- **Battery power is deferred until after bench testing**, not required to begin; future handheld operation stays portable and compact.
### LATER BATTERY OPTIONS — AMAZON SEARCH LINKS ONLY
- Main device compact USB-C 10,000mAh 5V power bank (example INIU; confirm output modes/current): https://www.amazon.com/s?k=INIU+10000mAh+5V+3A+power+bank
- Alternative 5,000mAh slim USB-C power bank: https://www.amazon.com/s?k=slim+5000mAh+USB+C+power+bank
- Remote stations: protected single 21700 4,000–5,000mAh cell + appropriately engineered charger/holder/regulator: https://www.amazon.com/s?k=protected+21700+5000mAh+rechargeable+battery
- Remote stations: smaller protected single 18650 2,500–3,500mAh cell + charger/holder/regulator: https://www.amazon.com/s?k=protected+18650+rechargeable+battery
- Optional single-cell flat 3.7V 2,000mAh LiPo, correct protection, polarity/connector, mechanical fit and charging limits for Waveshare PH2.0: https://www.amazon.com/s?k=3.7V+2000mAh+LiPo+PH2.0+battery
- Compact 5V 3A USB power bank category for remote relay testing if a one-cell design's runtime is insufficient: https://www.amazon.com/s?k=USB+C+power+bank+5V+3A
- All links are searches, not claims of current individual Amazon stock, cell authenticity, or electrical compatibility. No third-party storefront references needed for battery purchasing.
- HY2.0 3P/4P to Dupont compatible cable (first check what's included): https://www.amazon.com/s?k=HY2.0+4pin+Dupont+cable
- Jumper and breadboard: https://www.amazon.com/s?k=Dupont+jumper+wire+kit ; https://www.amazon.com/s?k=breadboard+prototype+PCB+kit
- 32GB microSD: https://www.amazon.com/s?k=32GB+microSD+card
- IP65 plastic enclosure & cable glands: https://www.amazon.com/s?k=IP65+plastic+electronic+project+box ; https://www.amazon.com/s?k=PG7+waterproof+cable+gland
- IMPORTANT USER REQUIREMENT: **No bulky 12V battery** for handheld or ordinary remote stations; prefer pocketable battery systems. A previous 12V suggestion is superseded.
- Handheld development: USB-C, 5V regulated power bank, 5,000-10,000mAh compact form factor, rated to sustain screen + sensor current: https://www.amazon.com/s?k=slim+10000mah+USB+C+power+bank
- Handheld future built-in battery: Waveshare PH2.0 2-pin accepts only ONE nominal 3.7V rechargeable lithium cell; manufacturer recommends <=2,000mAh for onboard CS8501 charge/discharge manager (580mA nominal charging). Follow https://docs.waveshare.com/ESP32-S3-Touch-LCD-7/Instructions-For-Use and https://docs.waveshare.com/ESP32-S3-Touch-LCD-7/FAQ . Verify battery pin polarity, protection and surge/radio load; do NOT parallel batteries at its battery port or connect 12V.
- Compact remote units: protected single 18650 ~2500-3500mAh or protected single 21700 ~4000-5000mAh nominal Li-ion cell, holder and matching charge/protection/voltage-regulation module. Alternative flat protected 1S 3.7V LiPo 1000-2000mAh. Search: https://www.amazon.com/s?k=protected+21700+rechargeable+battery+holder and https://www.amazon.com/s?k=protected+18650+battery+holder+charging+module
- Do NOT connect a raw 3.7V lithium cell directly to 5V input or a 3.3V-only ESP32 pin. USB 5V boost/regulator or correct 3.3V regulation mandatory. Choose the exact regulator/charger only after verifying the selected ESP32 board and sensor voltage requirements.
- Future multi-hop mesh: a sensor can report through neighboring ESP32 stations to an in-range relay and the handheld; preserve origin ID and hop/last-seen data, consider re-routing and outage buffering. ESP-NOW **alone is not automatically routed**; validate a suitable mesh stack/protocol on this screen + remote ESP32 variant. Mesh routers/relay stations must remain powered and listening for traffic; deep sleep invalidates mesh forwarding. Duty-cycle non-relay endpoints only when feature allows; measure real radio+radar consumption and battery life before making autonomy claims. Build battery state-of-charge and brownout telemetry, low-battery warnings and safe shutdown/reconnect logic.
- If continuous multi-day field operation is required, either use a larger-but-still-compact external USB power bank, solar charger designed for lithium chemistry, or mains where accessible; do not pretend a single small cell guarantees days of mesh relaying.
- Runtime estimates must use measured continuous and peak currents, battery Wh, converter efficiency, temperature and radio duty; do not invent hours.
- Remote battery runtime = battery Wh / measured average input W x real derating; no invented runtime before measuring.

## Physical limitations
Four orientations at 90-degree spacing give nominal horizontal overlap for >90-degree sensor beams; beam sensitivity is nonuniform, so blind spots require real tests. Different models have different vertical beams. A 24 GHz radar beam isn't a precise wall scanner; human body shadowing and multipath make walking, through-wall and through-floor results unreliable. Multiple same-band sensors may interfere. Thermal sensors detect surface emitted heat, not through normal walls. LiDAR maps optically visible surfaces, not hidden interior studs. GPS does not provide accurate indoor floor assignment; sensor location should show uncertainty. Orientation needs calibration.

## Updated final power form factor — removable 1S LiPo pouches (2026-10-07)
Final enclosure design should now prioritize **protected flat silver LiPo pouch packs**, rather than earlier cylindrical 18650/21700 options. Previous suggestions remain historical but are not the user's current preference. The main stays on 5V wall USB until hardware verification. Final remote stations get 2 individually isolated/removable protected LiPo battery cartridges; handheld gets 4; must support operation with any single adequate pack and unplugging another while running. Do NOT wire packs directly in parallel or into Waveshare PH2.0 built-in single-battery charger. Use appropriately rated multi-input ideal-diode OR/power mux, regulated 5V supply and independent cell charge/protection. Prefer external multi-bay charging dock first; check onboard independent charger channels later. Track each bay's SoC, SoH when gauge supports it, voltage, current, temperature, capacity, estimated minutes left, whether inserted/active, and transmit remote battery health over Wi-Fi/mesh. Full engineering and Amazon-only comparison queries: docs/HOT_SWAPPABLE_LIPO_POWER_SYSTEM.md.
