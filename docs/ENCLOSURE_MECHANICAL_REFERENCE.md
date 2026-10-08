# 3D-Printed Enclosure — Mechanical References and Initial Layout

Status: MECHANICAL DESIGN RESEARCH ONLY, 2026-10-07. No final printable case, STL, STEP or fit verification has been created. User has not yet physically measured/received/tested selected modules. All dimensions in millimeters unless noted.

## Waveshare 7-inch capacitive touchscreen
Model: Waveshare ESP32-S3-Touch-LCD-7 (SKU 27078, ESP32-S3, 800x480); not non-touch or 7B.
Official drawing: https://docs.waveshare.com/assets/images/ESP32-S3-Touch-LCD-7-Size-ef6da0acef8ae854412c4b14a866eeb5.webp
Official dimensional resources / original mechanical ZIP: https://docs.waveshare.com/ESP32-S3-Touch-LCD-7/Resources-And-Documents and https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-7/ESP32-S3-Touch-LCD-7.zip
Documentation: https://docs.waveshare.com/ESP32-S3-Touch-LCD-7

- Touch glass outline W192.96 H110.76, rounded corners.
- LCD illuminated/active rectangle W154.88 H86.72. The manufacturer front drawing gives x offset 19.04mm from left for active region. Top margin approximately 13.04mm in the manufacturer diagram; note that one extracted mirror image reverses top/bottom labeling. Orient from actual product to be certain. DO NOT cut a tight display-mask frame directly from uncertain top/bottom margin.
- Rear plate dimensioned W165.72 H97.60.
- Four rear mount **M3** hole locations make a horizontal center spacing of **126.20** and vertical center spacing of **65.65**.
- With rear plate upper-left as origin (X to right, Y down), derived M3 mounting coordinates based on the official drawing:
  top-left (19.76,18.31), top-right (145.96,18.31), bottom-left (19.76,83.96), bottom-right (145.96,83.96). Hence plate right margin 19.76 and bottom margin 13.64.
- Electronics-board **4×M2.5** hole pattern: 58.00 mm column spacing, 49.00 mm vertical row spacing; relative x offset 39.10 from M3 left column. Derived x centers 58.86 and 116.86 relative to plate left. The 5.85 vertical offset in drawing is between the lower M2.5 and M3 hole rows; tentative y centers 29.11 and 78.11 relative to plate top. CHECK orientation and physically measure before locking these in as printed posts.
- Mount using rear M3 supports for main case; avoid loading PCB / flex cable; verify M3 screw engagement depth and any existing metal threaded inserts on real unit.
- Assembly thickness, back protrusion and exact USB-C/SD, buttons, flex cable, mounting thread depth/clearance not sufficiently dimensioned to lock in depth before caliper measurement and/or import manufacturer's original 3D drawing.

## First three radar sensors (board outline)
1. Hi-Link LD2450: 44.00 × 15.00 mm PCB. Hi-Link technical figure 3; no standard four mounting holes illustrated, so initial clip cradle/carrier, nonmetallic mount or protected adhesive pad rather than invented screws. Check actual connector/headers and thickness. Datasheet https://www.rajguruelectronics.com/Product/30154/180625155314.00.pdf (see page 8).
2. DFRobot C4001 SEN0609, 25m: 26.00 × 30.00 mm; manufacturer dimension drawing https://dfimg.dfrobot.com/wiki/20522/SEN0609_gravity-c4001-24ghz-mmwave-human-presence-detection-sensor_dimension_V1.pdf . Two holes are centered near lower left/right corners; 22.00mm center-to-center horizontally and 2.00mm from lower edge to centers (also 2.00mm from each outer side edge). Hole DIAMETER not clearly called out: measure physically before printing tight screws. Connector 5-pin 2.54mm pitch.
3. DFRobot C4002 SEN0691: 22.00 × 26.00 mm from manufacturer https://wiki.dfrobot.com/sen0691/ . Product photo shows two mounting holes, but NO manufacturer-certified hole diameter or pitch confirmed; use slotted/adjustable clip until in hand. 5-pin edge connector/header; confirm pin and component protrusion height.

## Enclosure strategy: separate screen and sensor bar
- First case should be parametric and modular: display bezel/backshell, M3 mounting bosses, replaceable sensor brackets, battery bay with access and adequate cable routing, removable back/fasteners, accessible USB-C programming/power, BOOT and RESET, SD and UART ports.
- Sensor RF front faces must point OUTWARD, clear of metal backplate, batteries, circuit board shielding, screws, dense wiring, and hand/body blocking; transparent/nonconductive plastic radar windows where safe. Do not assume 24GHz modules can coexist with no interference; allow individual power and independent alignment.
- Mount prospective three radar modules at a separate top or side bracket, preferably detachable, so placement/spacing and direction can be tested without reprinting whole case. Keep antennas away from grip.
- If future thermal sensor added, ordinary opaque PLA/PETG enclosure will NOT provide a transmissive thermal-IR window; select correct optical/IR window material. If LiDAR added, use clear optical windows.
- Prototype shell exterior width on the order of 202–210mm and height 120–135mm is only a design allowance, not a final confirmed dimension. Thickness 25–40mm or more depends on chosen battery form factor, buttons and wiring and remains to be measured. Also consider top sensor bar width.
- Current bench power from a regulated USB wall adapter. Later handheld battery should be a compact 5,000–10,000mAh USB power bank or protected single cell with proper charger/regulator. Do not reserve huge 12V battery box; battery SKU/dimensions remain TBD.

## Fit-iteration
1. Confirm SKU/model and download manufacturer mechanical ZIP or display drawing.
2. Import model/drawing into a CAD package (FreeCAD/Fusion/Onshape as available). Use XY mounting references above.
3. Print only a low-material corner/boss fit test plate and bezel-edge gauge first. Check screws, hole centers, seating, glass clearance, grip and cable reach.
4. Measure actual module max component height and charging/power bank dimensions before modeling complete rear shell.
5. When fit is confirmed print removable back cover and clip-in sensor modules; record changes/test photos, do not claim mechanically VERIFIED beforehand.

## Battery cassette design supersedes initial generic power bank compartment (2026-10-07)
The user now specifically requests flat silver LiPo pouch cells (typically Kapton-taped) in replaceable protected cartridges: **TWO hot-swap bays per remote station and FOUR bays ideally spread near handheld corners**. The earlier generic USB bank option is superseded for the desired final mechanical design; initial wall USB power remains unchanged. Each bay requires guided keyed insertion, recessed short-safe power terminals, a mechanical latch/stop, connector alignment and rated contacts, strain relief, room for protection electronics, and relief against pouch compression, puncture or swelling. Cartridge connectors may use appropriately current-rated spring pogo mating contacts or a genuinely guided blind-mate connector; validate current rating and insertion durability. Battery must not touch printed sharp edges or mounting screws. Four-bay layout must leave adequate hand grips and screen rear plate support without blocking radar beam paths. Include independently managed charging dock as first design choice; onboard multi-channel charging later. **Do not finalize case dimensions** before selecting a protected actual pouch cell, matching connector/charger, PCB and measured sensor currents. Complete requirement and Amazon-only links: docs/HOT_SWAPPABLE_LIPO_POWER_SYSTEM.md.
