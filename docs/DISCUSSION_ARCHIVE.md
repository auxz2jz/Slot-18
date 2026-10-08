# Discussion Archive — retained design context
This file retains major requirements and constraints gathered before repository initialization (2026-10-07). Earlier ideas are proposals, not hardware test evidence.

## Starting point
User learned 24 GHz radar operates with a wavelength around 12.5 mm; “12 mm” refers approximately to wavelength, NOT a 12 mm object-detection threshold. Desired sensing: people moving, standing, sitting, sleeping; indoor and outdoor, distance and relative position; distinguish sensor outputs by capability.

Selected development platform: Waveshare ESP32-S3-Touch-LCD-7 7-inch capacitive display with built-in ESP32-S3. Human target tracker: Hi-Link LD2450 (up to 3 moving tracks, XY planar position; not reliably stationary and not object classification). Long range: DFRobot C4001 SEN0609, up to nominal 25 m moving / 16 m stationary presence. Stationary: C4002 SEN0691. Potential SC16IS752 I2C 2xUART bridge; C4001 published I2C interface requires connector verification.

## Geometry and mobility
Original idea: arrange three different radars in each of four cardinal directions, later 8 directions if overlap insufficient. Four sectors spaced 90 degrees apart can nominally cover 360 degrees when each sensor's horizontal beam exceeds 90 degrees; real beam edges, orientation and mechanical blocking can cause gaps. Do not buy 24 modules based on schematic coverage alone. Another concept was 4 x 3 = 12 sensors versus 8 x 3 = 24 sensors. Each type has different vertical beam; upward-facing radar can indicate elevated detection, but 2D radar cannot measure independent elevation or floor assignment. A person on floor 3 behind walls/floors is NOT a reliable detectable target. Person holding screen can shadow antennas and appear as target. Plan optional elevated pole and walking/stationary modes.

## Physical connection
ESP32-S3 processing normalized 24-device reports is plausibly manageable, but adding 24 separate serial connections to one board is not straightforward. Prefer distributed microcontrollers per direction or a tested expansion bus; eight small ESP32 nodes could forward aggregated observations. Wired RS485 and wireless ESP-NOW/local Wi-Fi are both options. Keep one 7-inch master screen, not five copies. No cloud required.

## Remote stations and mapping
User wants five or more independently battery-powered stations visible together from above. A station's GNSS/map coordinates and calibrated heading/tilt define its sensor pose. Radar XY plus station position/pose MAY support approximate world-coordinate plots with uncertainty; presence or range-only cannot determine exact person coordinates. Standard GNSS outdoor position is often off by meters. High precision needs RTK corrections (e.g. ZED-F9P); GNSS stationary heading is generally unavailable, so BNO085+calibrated heading/magnetometer or known fixed mounts are options. Need map/floorplan registration and known heights.

## Other modalities discussed
- Thermal AMG8833 8x8 or MLX90640 32x24; detects thermal surfaces, not through walls.
- LiDAR (RPLIDAR A1) 2D overhead room outlines, not hidden wall cavities, no seeing through walls.
- VL53L1X ToF, VL53L5CX multizone depth, ultrasonic, PIR, reed switch, ADXL345 vibration, GPS/compass, air/environment sensing.
- Camera can help classify car/person/animal with sufficient compute and consent/privacy, but normal 24 GHz modules cannot classify reliably.
- High-end true 3D radar (TI IWR6843) may return range/azimuth/elevation; more cost/compute.
- Specialist wall scanners may detect studs/metal/plumbing within construction; different technology, no guarantee people behind walls.
- Sensing cars: motion/echo possible within specs, but not dependable vehicle class; LiDAR/radar/camera complement.

## Battery and power
All components battery-powered, including master touchscreen and remote stations. Begin USB 5V power bank for lab. Later choose protected rechargeable battery, correct BMS/charger and 5V DC supply. Measure runtime: continuous radar + display + GNSS + radios are substantial loads. Outdoor stations require weather-resistant plastic radar window and thermal/optical sensor-specific windows, appropriate water ingress and condensation mitigation.

## Shopping and software approach
Manufacturer models and Amazon model-name searches stored in docs/HARDWARE_AND_SENSORS.md. Purchase first one of each intended sensor (not 12 or 24), screen, connection cables and power; check UART bridge requirements and compatibility before bulk orders.
Development agreed: start UI + normalized sample data without hardware, then port proven Waveshare driver/ESP-IDF LVGL demo, then decode LD2450, C4001, C4002 individually and together, record diagnostics, guided tests, configuration and a last user-verified checkpoint. Initial target is v0.1.0 software-only candidate, not flashed ESP32 firmware.
